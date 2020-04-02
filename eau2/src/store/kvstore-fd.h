#pragma once
// lang: CwC

#include "../client/arg.h"
#include "../client/network.h"
#include "../utils/map.h"
#include "kv.h"

/** Forward declaration of DataFrame. **/
class DataFrame;

/** Generates a non-concurrent KV-map **/
generate_object_classarray(KeyArray, Key);
generate_object_classarray(ValueArray, Value);
generate_classmap(KVMap, KVNode, KeyArray, ValueArray, Key *, Value *);

/** Represents a concurrent map from Keys to size_t. **/
class ConcurrentKVMap : public KVMap {
public:
  Lock lock_;

  void put(Key *k, Value *v) {
    lock_.lock();
    KVMap::put(k, v);
    lock_.unlock();
  }

  Value *get(Key *k) {
    lock_.lock();
    Value *hold = KVMap::get(k);
    lock_.unlock();
    return hold;
  }
};

/** Forward declaration of KVStore servicer. **/
class KVStoreServicer;

/** Represents a Key-Value Store from a network. **/
class KVStore : public ConcurrentKVMap {
public:
  size_t index_;
  Network *network_ = nullptr;
  KVStoreServicer *servicer_ = nullptr;
  ConcurrentMessageQueue replies;

  /** Creates a KVStore at a given index and with a given network. **/
  KVStore(size_t index, Network *network) : index_(index), network_(network) {}
  ~KVStore() {
    KeyArray *ks = keys();
    for (size_t i = 0; i < ks->size(); i++) {
      delete remove(ks->get(i));
    }
    delete ks;
  }

  /** Gets the index of the keyvalue store **/
  size_t index() { return index_; }

  /** Gets a distributed dataframe that is locally hosted on this KV store.  **/
  DataFrame *get(Key *key);

  /** Gets a distributed dataframe that is *not* hosted at this store. **/
  DataFrame *get_and_wait(Key *key);

  /** Stores a key and value at the desired node. **/
  void put(Key *key, Value *value);

  /** Private methods on the Key-Value store that return Values. **/
  Value *get_value(Key *key) { return ConcurrentKVMap::get(key); }
  Value *get_and_wait_value(Key *key);

  /** Starts/stops a thread that services incoming requests on the network. **/
  void start_service();
  void stop_service();
  void wait_to_close();
};