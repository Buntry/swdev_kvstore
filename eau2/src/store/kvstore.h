#pragma once
// lang: CwC

#include "../client/network.h"
#include "../utils/map.h"
#include "dataframe.h"

/** Gets a distributed dataframe that is locally hosted on this KV store.  **/
DataFrame *KVStore::get(Key *key) {
  assert(key->node() == index_ && contains_key(key));

  // Load data from local storage.
  Value *from = KVMap::get(key);
  Deserializer dser(*from->blob());
  Schema *schema = Schema::deserialize(dser);

  // Let this dataframe know about its actual dimensions
  DataFrame *df = new DataFrame(*schema, this);
  df->set_distributed_schema_(key, schema);
  df->must_load_on_next_query_();
  return df;
}

/** Gets a distributed dataframe that is *not* hosted at this store. **/
DataFrame *KVStore::get_and_wait(Key *key) {
  if (key->node() == index_)
    return get(key);

  // Load data from off this node
  Value *from = get_and_wait_value(key);
  Deserializer dser(*from->blob());
  delete from;

  Schema *schema = Schema::deserialize(dser);

  // Let this dataframe know about its actual dimensions.
  DataFrame *df = new DataFrame(*schema, this);
  df->set_distributed_schema_(key, schema);
  return df;
}

/** Stores a key and value at the desired node. **/
void KVStore::put(Key *key, Value *value) {
  printf("PUT K(%s) -> (%d)\n", key->key()->c_str(), (int)key->node());
  if (key->node() == index_) {
    return KVMap::put(key, value);
  } else {
    assert(false && "Tried to send a message off this node");
  }
}

/** Reaches across the network and acquires a value from another node. **/
Value *KVStore::get_and_wait_value(Key *key) {
  if (key->node() == index_)
    return get_value(key);

  assert(false && "Tried to reach a message off this node");
}
