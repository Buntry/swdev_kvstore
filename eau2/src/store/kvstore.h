#pragma once
// lang: CwC

#include "../client/network.h"
#include "../utils/map.h"
#include "dataframe.h"

/** Gets a distributed dataframe that is locally hosted on this KV store.  **/
DataFrame *KVStore::get(Key *key) {
  assert(key->node() == index_ && contains_key(key));
  Value *from = KVMap::get(key);
  Deserializer dser(*from->blob());
  Schema *schema = Schema::deserialize(dser);
  return new DataFrame(*schema, this);
}

/** Gets a distributed dataframe that is *not* hosted at this store. **/
DataFrame *KVStore::get_and_wait(Key *key) {
  if (key->node() == index_)
    return get(key);

  return nullptr;
}

/** Stores a key and value at the desired node. **/
void KVStore::put(Key *key, Value *value) {}
