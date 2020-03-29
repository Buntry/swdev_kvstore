#pragma once
// lang: CwC

#include "../store/kvstore.h"
#include "arg.h"
#include "network.h"

/** Represents a distributed application.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Application : public Object {
public:
  size_t index_;
  Network *network_ = nullptr;
  KVStore store_;

  /** Creates an application with from the given network. **/
  Application(size_t index, Network *network)
      : index_(index), network_(network), store_(index, network) {}

  /** Returns a read-only copy of this node's KV store/idx/net. **/
  KVStore *this_store() { return &store_; }
  size_t this_node() { return index_; }
  Network *network() { return network_; }

  /** Runs this application, meant to be overridded. **/
  virtual void run_() { assert(false); }

  /** Starts this application. **/
  void start() {
    store_.start_service();
    run_();
  }

  /** Stops this application. **/
  void stop() { store_.stop_service(); }
};