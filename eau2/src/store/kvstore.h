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
  df->must_load_on_next_query_();
  return df;
}

/** Another thread whose sole job is to check to see if the given key has
 * arrived.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class KVStoreReplier : public Thread {
public:
  size_t index_;
  KVStore *store_;
  Network *network_;
  Get *get_;

  KVStoreReplier(size_t index, KVStore *store, Network *network, Get *get)
      : index_(index), store_(store), network_(network), get_(get) {}
  ~KVStoreReplier() { delete get_; }

  void run() {
    while (!store_->contains_key(get_->key())) {
      Thread::sleep(10);
    }
    Value *value = store_->get_value(get_->key());
    Reply *rep = new Reply(get_->key()->clone(), value->clone());
    rep->init(index_, get_->sender(), 0);
    network_->send_msg(rep);
  }
};
generate_object_classarray(KVStoreReplierArray, KVStoreReplier);

/** Represents a thread that listens and services requests for a KVStore.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class KVStoreServicer : public Thread {
public:
  size_t index_;
  KVStore *store_;
  Network *network_;
  Lock lock_;
  Value *cur_value_ = nullptr;
  KVStoreReplierArray repliers;

  KVStoreServicer(size_t index, KVStore *store, Network *network)
      : index_(index), store_(store), network_(network) {}
  ~KVStoreServicer() {
    for (size_t i = 0; i < repliers.size(); i++) {
      KVStoreReplier *replier = repliers.get(i);
      replier->join();
      delete replier;
    }
  }

  /** Begins the service thread. **/
  void run() {
    network_->register_node(index_);
    while (true) {
      Message *msg = network_->receive_msg();
      switch (msg->kind()) {
      case MsgKind::Put:
        handle_put(dynamic_cast<Put *>(msg));
        break;
      case MsgKind::Get:
        handle_get(dynamic_cast<Get *>(msg));
        break;
      case MsgKind::Reply:
        handle_reply(dynamic_cast<Reply *>(msg));
        break;
      case MsgKind::Kill:
        delete msg;
        return;
      case MsgKind::Status:
        break;
      case MsgKind::Register:
        break;
      case MsgKind::Directory:
        break;
      default:
        break;
      }
    }
  }

  /** Get the next value. **/
  Value *next_value() {
    lock_.lock();
    while (cur_value_ == nullptr) {
      lock_.wait();
    }
    assert(cur_value_ != nullptr);
    Value *hold = cur_value_;
    cur_value_ = nullptr;
    lock_.unlock();
    return hold;
  }

  /** Handles reception of a put message by placing the key and value in local
   * storage. **/
  void handle_put(Put *put) {
    assert(put->target() == index_);
    assert(put->key()->node() == index_);
    store_->put(put->key(), put->value()->clone());
    delete put;
  }

  /** Handles reception of a get message by replying back with the data of the
   * message. **/
  void handle_get(Get *get) {
    assert(get->target() == index_);
    assert(get->key()->node() == index_);
    KVStoreReplier *r = new KVStoreReplier(index_, store_, network_, get);
    r->start();
    repliers.push_back(r);
  }

  /** Handles the reception of a reply message by adding it to the queue. **/
  void handle_reply(Reply *reply) {
    lock_.lock();
    cur_value_ = reply->value()->clone();
    lock_.notify_all();
    lock_.unlock();
    delete reply;
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
}

/** Waits on the servicer thread to close. This is done when the servicer
 * receives a kill message. **/
void KVStore::wait_to_close() {
  servicer_->join();
  delete servicer_;
}

/** Stores a key and value at the desired node. **/
void KVStore::put(Key *key, Value *value) {
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

  return servicer_->next_value();
}
