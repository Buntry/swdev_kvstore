#pragma once
// lang: CwC

#include "../client/network.h"
#include "../utils/map.h"

/** Forward declaration of DataFrame. **/
class DataFrame;

/** A Key that represents where data is stored. **/
class Key : public Object {
public:
  String *key_; // Represents the name of the key
  size_t node_; // Represents the location of where the data is

  /** Various ways to construct a Key **/
  Key(String &key, size_t node) : key_(key.clone()), node_(node) {}
  Key(String *key, size_t node) : key_(key), node_(node) {}
  Key(const char *key, size_t node) : key_(new String(key)), node_(node) {}
  Key(Key &k) : key_(k.key_->clone()), node_(k.node_) {}

  /** Hash/Clone functions for storage in a map **/
  size_t hash() { return key_->hash() ^ node_; }
  Key *clone() { return new Key(*this); }

  /** Serializes a key **/
  void serialize(Serializer &ser) {
    ser.write(key_);
    ser.write(node_);
  }

  /** Deserializes a key **/
  static Key *deserialize(Deserializer &dser) {
    String *k = String::deserialize(dser);
    size_t n = dser.read_size_t();
    return new Key(k, n);
  }

  /** Getters for a key **/
  String *key() { return key_; }
  size_t node() { return node_; }
};

/** A Value is a serialized blob of data. **/
class Value : public Object {
public:
  CharArray *blob_;

  /** Values are initialized with all of the data. **/
  Value(CharArray *blob) : blob_(blob) {}
  Value(CharArray &blob) : blob_(blob.clone()) {}
  Value(Deserializer &dser) : blob_(dser.data()->clone()) {}
  ~Value() { delete blob_; }

  /** Returns the blob of data. **/
  CharArray *blob() { return blob_; }

  /** Serializes a value into a serializer. **/
  void serialize(Serializer &ser) {
    ser.write(blob()->size());
    for (size_t i = 0; i < blob()->size(); i++) {
      ser.write(blob()->get(i));
    }
  }

  /** Deserializes a value from a deserializer. **/
  static Value *deserialize(Deserializer &dser) {
    size_t len = dser.read_size_t();
    CharArray *blob = new CharArray();
    for (size_t i = 0; i < len; i++) {
      blob->push_back(dser.read_char());
    }
    return new Value(blob);
  }
};

/** Generates a non-concurrent KV-map **/
generate_object_classarray(KeyArray, Key);
generate_object_classarray(ValueArray, Value);
generate_classmap(KVMap, KVNode, KeyArray, ValueArray, Key *, Value *);

/** Represents a Key-Value Store from a network. **/
class KVStore : public KVMap {
public:
  size_t index_;
  Network *network_ = nullptr;

  /** Creates a KVStore at a given index and with a given network. **/
  KVStore(size_t index, Network *network) : index_(index), network_(network) {}

  /** Gets a distributed dataframe that is locally hosted on this KV store.  **/
  DataFrame *get(Key *key);
  // assert(key->node() == index_ && contains_key(key));
  // Value *from = KVMap::get(key);
  // Deserializer dser(*from->blob());
  // Schema *schema = Schema::deserialize(dser);
  // return new DataFrame(*schema, this);

  /** Gets a distributed dataframe that is *not* hosted at this store. **/
  DataFrame *get_and_wait(Key *key);
  // if (key->node() == index_)
  //   return get(key);

  // return nullptr;

  /** Stores a key and value at the desired node. **/
  void put(Key *key, Value *value);
};