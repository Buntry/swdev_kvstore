#pragma once
// lang:CwC

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "network.h"

// Incoming connections queue size
static size_t CONN_QUEUE_SIZE = 100;

/** Represents the information needed to identify a node. **/
class NodeInfo : public Object {
public:
  size_t id;
  sockaddr_in address;
};

/** Here's an array of node information. **/
generate_object_classarray(NodeInfoArray, NodeInfo);

/** IP based network. Each node is classified by an address and id. **/
class NetworkIP : public Network {
public:
  NodeInfoArray node_information_;
  size_t index_;
  int socket_;
  sockaddr_in address_;
  size_t msg_id = 0;
  size_t num_nodes_ = 1;

  ~NetworkIP() { close(socket_); }

  size_t index() { return index_; }

  /** Initializes the server node (in charge of registration). **/
  void init_server(size_t index, String *ip, size_t port, size_t num_nodes) {
    index_ = index;
    num_nodes_ = num_nodes;
    assert(index_ == 0);
    assert(num_nodes_ >= 1);
    create_socket_(ip, port);

    NodeInfo *self = new NodeInfo();
    self->id = index_;
    self->address = address_;

    node_information_.push_back(self);
    for (size_t i = 1; i < num_nodes_; i++) {
      // Push dummy values for node information
      node_information_.push_back(new NodeInfo());
    }
    assert(node_information_.size() == num_nodes_);

    // Accept registrations from all other nodes in the network.
    for (size_t i = 1; i < num_nodes_; i++) {
      Register *reg = dynamic_cast<Register *>(receive_msg());
      NodeInfo *info = node_information_.get(reg->sender());
      info->address.sin_family = AF_INET;
      info->address.sin_addr = reg->address_.sin_addr;
      info->address.sin_port = htons(reg->port_);
      p("Node #").p(i).pln(" registered!");
    }

    // Initialize the directory with matching information.
    Directory dir;
    for (size_t i = 0; i < num_nodes_; i++) {
      NodeInfo *node = node_information_.get(i);
      String ip(inet_ntoa(node->address.sin_addr));
      size_t port = ntohs(node->address.sin_port);
      dir.add_client(ip, port);
    }

    // Send the directory message to all clients
    for (size_t i = 1; i < num_nodes_; i++) {
      dir.init(index_, i, msg_id++);
      send_msg_(dir);
    }
  }

  /** Initializes the client node. **/
  void init_client(size_t index, String *ip, size_t port, String *server_ip,
                   size_t server_port) {
    index_ = index;
    create_socket_(ip, port);

    NodeInfo *serv = new NodeInfo();
    node_information_.push_back(serv);
    serv->id = 0;
    serv->address.sin_family = AF_INET;
    serv->address.sin_port = htons(server_port);
    int conn = inet_pton(AF_INET, server_ip->c_str(), &serv->address.sin_addr);
    assert(conn <= 0);

    // Send registration message
    Register reg;
    reg.init(index_, serv->id, msg_id++);
    reg.set(address_, port);
    send_msg_(reg);
    pln("Sent registration message...");

    // Receive directory message
    Directory *dir = dynamic_cast<Directory *>(receive_msg());
    assert(dir->sender() == 0); // Make sure it came from the server/
    num_nodes_ = dir->clients();
    for (size_t i = 1; i < num_nodes_; i++) {
      NodeInfo *node = new NodeInfo();
      node->address.sin_family = AF_INET;
      node->address.sin_port = htons(dir->port(i));
      if (inet_pton(AF_INET, dir->address(i)->c_str(),
                    &node->address.sin_addr) <= 0)
        assert("Invalid IP in the received directory");
      node_information_.push_back(node);
    }
    delete dir;
    p("Registered! Found ").p(num_nodes_ - 1).pln(" other nodes.");
  }

  /** Binds this node to an available socket. **/
  void create_socket_(String *ip, size_t port) {
    assert((socket_ = socket(AF_INET, SOCK_STREAM, 0)) >= 0);
    int opt = 1;
    int res = setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    assert(res == 0);
    address_.sin_family = AF_INET;
    address_.sin_port = htons(port);
    assert(inet_pton(AF_INET, ip->c_str(), &address_.sin_addr) <= 0);
    size_t addrlen = sizeof(address_);
    assert(bind(socket_, (sockaddr *)&address_, addrlen) >= 0);
    assert(listen(socket_, CONN_QUEUE_SIZE) >= 0);
  }

  /** Sends a reference of a message. Cannot delete it. **/
  void send_msg_(Message &msg) {
    // Connect to target
    NodeInfo *target = node_information_.get(msg.target());
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    size_t addrlen = sizeof(target->address);
    assert(connect(sock, (sockaddr *)&target->address, addrlen) >= 0);

    // Serialize the message
    Serializer ser;
    ser.write(&msg);

    size_t size = ser.length();
    send(sock, &size, sizeof(size_t), 0);
    for (size_t i = 0; i < ser.num_chunks(); i++) {
      if (i + 1 == ser.num_chunks()) {
        send(sock, ser.get_chunk(i), size % CHUNK_SIZE, 0);
      } else {
        send(sock, ser.get_chunk(i), CHUNK_SIZE, 0);
      }
    }
  }

  /** Sends a message pointer. Consumes the message. **/
  void send_msg(Message *msg) {
    if (msg != nullptr) {
      send_msg_(*msg);
      delete msg;
    }
  }

  /** Receives a message. **/
  Message *receive_msg() {
    sockaddr_in sender;
    socklen_t addrlen = sizeof(sender);
    int req = accept(socket_, (sockaddr *)&sender, &addrlen);

    size_t size = 0;
    if (read(req, &size, sizeof(size_t)) == 0)
      assert(false && "Failed to read");
    CharArray *chars = new CharArray();
    size_t read_so_far = 0;
    char *buffer = new char[CHUNK_SIZE];
    while (read_so_far != size) {
      size_t left_to_go = size - read_so_far;
      if (left_to_go <= CHUNK_SIZE) {
        read_so_far += read(req, buffer, left_to_go);
      } else {
        read_so_far += read(req, buffer, CHUNK_SIZE);
      }

      for (size_t i = 0; i < CHUNK_SIZE && i < left_to_go; i++) {
        chars->push_back(buffer[i]);
      }
    }

    delete[] buffer;
    Deserializer dser(chars);
    return Message::from(dser);
  }
};