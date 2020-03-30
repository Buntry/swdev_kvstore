#pragma once
// lang: CwC

#include "../utils/map.h"
#include "network.h"

/** Mapping from String to size_t **/
generate_classmap(SSTMap, SSTNode, StringArray, SizeTArray, String *, size_t);

/** Represents a concurrent map from String to size_t. **/
class ConcurrentSSTMap : public SSTMap {
public:
  Lock lock_;

  void put(String *k, size_t v) {
    lock_.lock();
    SSTMap::put(k, v);
    lock_.unlock();
  }

  size_t get(String *k) {
    lock_.lock();
    size_t hold = SSTMap::get(k);
    lock_.unlock();
    return hold;
  }
};

/** Here's an array of concurrent message queues. **/
generate_object_classarray(MessageQueueArray, ConcurrentMessageQueue);

/** Pseudo Network Layer. The network comprises several threads on the same
 * machine. Network messages are never deleted. **/
class NetworkPseudo : public Network {
public:
  ConcurrentSSTMap threads_;
  MessageQueueArray queues_;
  size_t num_nodes_;

  NetworkPseudo(size_t num_nodes) : num_nodes_(num_nodes) {
    for (size_t i = 0; i < num_nodes_; i++) {
      queues_.push_back(new ConcurrentMessageQueue());
    }
  }

  ~NetworkPseudo() {
    for (size_t i = 0; i < num_nodes_; i++) {
      delete queues_.get(i);
    }
  }

  /** Registers a node with the rendezvous server. **/
  void register_node(size_t idx) {
    String *tid = Thread::thread_id();
    threads_.put(tid, idx);
    delete tid;
  };

  /** Gets the index of the current node. **/
  virtual size_t index() {
    String *tid = Thread::thread_id();
    size_t ret = threads_.get(tid);
    delete tid;
    return ret;
  }

  /** Sends a message (to the message's target). **/
  virtual void send_msg(Message *m) {
    // We should delete message
    queues_.get(m->target_)->push(m);
  }

  /** Receives a message **/
  virtual Message *receive_msg() {
    String *tid = Thread::thread_id();
    size_t idx = threads_.get(tid);
    delete tid;
    return queues_.get(idx)->pop();
  }
};