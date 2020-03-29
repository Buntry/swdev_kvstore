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

/** Represents a thread that listens and services requests for a KVStore.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class KVStoreServicer : public Thread {
public:
  size_t index_;
  KVStore *store_;
  Network *network_;

  KVStoreServicer(size_t index, KVStore *store, Network *network)
      : index_(index), store_(store), network_(network) {}

  /** Begins the service thread. **/
  void run() {
    network_->register_node(index_);
    while (true) {
      Message *msg = network_->receive_msg();
      p("Received a message! ").pln(static_cast<size_t>(msg->kind()));
      switch (msg->kind()) {
      case MsgKind::Kill:
        delete msg;
        return;
        break;
      case MsgKind::Put:
        handle_put(dynamic_cast<Put *>(msg));
        break;
      case MsgKind::Get:
        handle_get(dynamic_cast<Get *>(msg));
        break;
      case MsgKind::Reply:
        handle_reply(dynamic_cast<Reply *>(msg));
        break;
      case MsgKind::Status:
        break;
      case MsgKind::Register:
        break;
      case MsgKind::Directory:
        break;
      default:
        break;
      }
      delete msg;
    }
  }

  /** Handles reception of a put message by placing the key and value in local
   * storage. **/
  void handle_put(Put *put) {
    assert(put->target() == index_);
    assert(put->key()->node() == index_);
    store_->put(put->key(), put->value()->clone());
  }

  /** Handles reception of a get message by replying back with the data of the
   * message. **/
  void handle_get(Get *get) {
    assert(get->target() == index_);
    assert(get->key()->node() == index_);
    while (!store_->contains_key(get->key())) {
      sleep(10);
    }
    Value *value = store_->get_value(get->key());
    Reply *rep = new Reply(get->key()->clone(), value->clone());
    rep->init(index_, get->sender(), 0);
    network_->send_msg(rep);
    delete value;
  }

  /** Handles the reception of a reply message by adding it to the queue. **/
  void handle_reply(Reply *reply) {
    assert(reply->target() == index_);
    store_->replies.push(reply);
    p("PUSHED REPLY @")
        .p(index_)
        .p(" with K(")
        .p(reply->key()->key()->c_str())
        .p(") from (")
        .p(reply->sender())
        .pln(")");
    assert(store_->replies.size() > 0);
  }
};

/** Starts a thread that services incoming requests on the network. **/
void KVStore::start_service() {
  servicer_ = new KVStoreServicer(index_, this, network_);
  servicer_->start();
}

/** Stops a thread by sending itself a kill message **/
void KVStore::stop_service() {
  Message *kill = new Kill();
  kill->init(index_, index_, 0);
  network_->send_msg(kill);
  servicer_->join();
  delete servicer_;
}

/** Stores a key and value at the desired node. **/
void KVStore::put(Key *key, Value *value) {
  printf("PUT K(%s) from (%d) -> (%d)\n", key->key()->c_str(), (int)index_,
         (int)key->node());
  if (key->node() == index_)
    return ConcurrentKVMap::put(key, value);

  Message *put = new Put(key->clone(), value);
  put->init(index_, key->node(), 0);
  network_->send_msg(put);
}

/** Reaches across the network and acquires a value from another node. **/
Value *KVStore::get_and_wait_value(Key *key) {
  if (key->node() == index_)
    return get_value(key);

  Message *get = new Get(key->clone());
  get->init(index_, key->node(), 0);
  network_->send_msg(get);

  Reply *reply = dynamic_cast<Reply *>(replies.pop());
  Value *to_return = reply->value()->clone();
  delete reply;
  return to_return;
}
