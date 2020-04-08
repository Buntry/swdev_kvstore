#pragma once
// lang: CwC

#include "../utils/map.h"

/** A Key that represents where data is stored. **/
class Key : public Object {
public:
  String *key_; // Represents the name of the key
  size_t node_; // Represents the location of where the data is

  /** Various ways to construct a Key **/
  Key(String &key, size_t node) : key_(key.clone()), node_(node) {}
  Key(String *key, size_t node) : key_(key), node_(node) {}
  Key(String *key) : key_(key), node_(0) {}
  Key(const char *key) : key_(new String(key)), node_(0) {}
  Key(const char *key, size_t node) : key_(new String(key)), node_(node) {}
  Key(Key &k) : key_(k.key_->clone()), node_(k.node_) {}
  ~Key() { delete key_; }

  /** Hash/Clone functions for storage in a map **/
  bool equals(Object *other) {
    Key *that = dynamic_cast<Key *>(other);
    return (that != nullptr) && (this->key()->equals(that->key())) &&
           (this->node() == that->node());
  }
  size_t hash() { return key_->hash(); }
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
  size_t size() { return blob_->size(); }
  Value *clone() { return new Value(*blob()); }

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