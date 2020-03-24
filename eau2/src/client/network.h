// lang: CwC
#pragma once
#include "message.h"

/**
 * @brief Represents a network interface.
 */
class Network : public Object {
public:
  /** Registers a node with the rendezvous server. **/
  virtual void register_node(size_t idx){};
  /** Gets the index of the current node. **/
  virtual size_t index() { return 0; }

  /** Sends a message (to the message's target). **/
  virtual void send_msg(Message *m) {}
  /** Receives a message **/
  virtual Message *receive_msg() { return nullptr; }
};