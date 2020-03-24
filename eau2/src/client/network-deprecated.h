// lang: CwC
#pragma once

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "map.h"
#include "queue.h"
#include "serializer.h"
#include "thread.h"

static const short PORT = 8080;         // Port
static const size_t QUEUE_SIZE = 10;    // Max # of inc. requests
static const size_t MSG_SIZE = 1024;    // Message size
static const size_t ALL_CONNECT = 2000; // Wait for all nodes to connect

/** Represents a Queue of Messages.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
generate_object_classqueue(MessageQueue, Message);

/** Represents a thread that listens to a socket and appends read messages
 * to a message queue.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class SocketListener : public Thread {
public:
  int socket_;
  MessageQueue *mq_;
  Lock *lock_;
  bool running = true;

  SocketListener(int socket, MessageQueue *mq, Lock *lock)
      : socket_(socket), mq_(mq), lock_(lock) {}

  /** Infinitely loops on this socket, appending input to the queue. **/
  void run() {
    char message[MSG_SIZE] = {0};
    while (running) {
      while (read(socket_, message, MSG_SIZE) > 0) {
        lock_->lock();
        char *buf = reinterpret_cast<char *>(message);
        mq_->push(message_from(buf));
        lock_->unlock();
      }
    }
  }

  /** Turns off the infinite loop. **/
  void off() { running = false; }
};

/** Represents an array of socket listeners.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class SocketListenerArray : public Array {
public:
  SocketListener *get(size_t i) {
    return dynamic_cast<SocketListener *>(Array::get(i));
  }
};

/** Boxed Primitive for a Socket **/
class Socket : public Object {
public:
  int sock_;
  int get() { return sock_; }
  Socket(int sock) : sock_(sock) {}
};

/** Map from IPs to Sockets **/
class SocketMap : public Map {
public:
  Socket *get(String *ip, int port) {
    StrBuff sb;
    String *k = sb.c(*ip).c(":").c(port).get();
    Socket *res = dynamic_cast<Socket *>(Map::get(k));
    delete k;
    return res;
  }

  void put(String *ip, int port, int socket) {
    StrBuff sb;
    sb.c(*ip).c(":").c(port);
    Map::put(sb.get(), new Socket(socket));
  }

  bool contains_key(String *ip, int port) {
    StrBuff sb;
    String *k = sb.c(*ip).c(":").c(port).get();
    bool res = Map::contains_key(k);
    delete k;
    return res;
  }
};

/** Represents a thread that accepts connections.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class AcceptConnection : public Thread {
public:
  int id_;
  int file_descriptor;
  int *curr;
  size_t max_connections;
  MessageQueue *message_queue_;
  SocketMap *socket_map_;
  SocketListenerArray sla_;
  Lock *lock_;

  /** Sets this object with a number of connections **/
  void set(int id, int fd, int *c, size_t m, SocketMap *sm, MessageQueue *mq,
           Lock *l) {
    id_ = id;
    curr = c;
    file_descriptor = fd;
    max_connections = m;
    socket_map_ = sm;
    message_queue_ = mq;
    lock_ = l;
  }

  /** Accept connection and setup a listener. **/
  void accept_connection() {
    struct sockaddr_in client;
    size_t clientlen = sizeof(client);
    assert((*curr = accept(file_descriptor, (struct sockaddr *)&client,
                           (socklen_t *)&clientlen)) >= 0);
  }

  /** Sets up the listeners. **/
  void run() {
    char message[MSG_SIZE] = {0};
    for (size_t i = 0; i < max_connections; i++) {
      accept_connection();
      lock_->lock();
      if (id_ == 0) {
        read(*curr, message, MSG_SIZE);
        char *buf = reinterpret_cast<char *>(message);
        Message *m = message_from(buf);
        m->init(m->sender_, m->target_, *curr);
        message_queue_->push(m);
      }
      SocketListener *sl = new SocketListener(*curr, message_queue_, lock_);
      sl->start();
      sla_.push_back(sl);
      lock_->unlock();
    }
  }
};

/** Represents a Node in a distributed application.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Node : public Object {
public:
  int file_descriptor;        // This socket's file descriptor
  int curr;                   // The current connection's descriptor
  struct sockaddr_in address; // This address (ip/port)
  int opt = 1;                // Used for setsockopt

  int id_; // Id of this node.

  Directory dir;    // Directory of all seen nodes.
  String *ip_;      // ip of this address
  int port_;        // port of this address
  size_t num_nodes; // Number of nodes in this network (might not be known)

  size_t cur_message_id = 0;         // Current id of the message we send
  size_t looking_for_message_id = 0; // An id for a message to look for

  MessageQueue inbound;  // Messages this node is receiving
  MessageQueue outbound; // Messages this node is sending
  SocketMap socket_map;  // Active sockets
  AcceptConnection ac;   // Thread for accepting some amt of conns
  Lock lock;             // General lock to force sequential ops

  /** Starts up the node at the given ip. Forcefully attaches to a socket. **/
  Node(const char *ip, int port) {
    ip_ = new String(ip);
    port_ = port;
    id_ = 0;

    // Add ourselves to the directory.
    dir.add(ip_->clone(), port_);

    // Setup connection.
    startup_();
  }

  /** Starts up the node at the given ip, waits on connections from n nodes. **/
  Node(const char *ip, int port, size_t n_waitfor) : Node(ip, port) {
    if (n_waitfor > 0) {
      num_nodes = n_waitfor + 1;
      accept_connections(num_nodes);
    }
  }

  /** Node destructor **/
  ~Node() { delete ip_; }

  /** Starts up this node on an available socket. Commences listening. **/
  void startup_() {
    // Creates the socket file descriptor
    assert((file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) != 0);

    // Add ourselves to the socket map.
    socket_map.put(ip_, port_, file_descriptor);

    // Attach to a socket
    setsockopt(file_descriptor, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
               sizeof(opt));
    // Set up this address
    format_addr_(address, ip_, port_);

    // Binding socket to the port
    assert(bind(file_descriptor, (struct sockaddr *)&address,
                sizeof(address)) >= 0);

    // Listen for a maxmimum number of clients.
    assert(listen(file_descriptor, QUEUE_SIZE) >= 0);
  }

  /** Formats an IP and port into a sockaddr_in struct. **/
  void format_addr_(sockaddr_in &addr, String *ip, int port) {
    addr.sin_family = AF_INET;
    assert(inet_pton(AF_INET, ip->c_str(), &addr.sin_addr) == 1);
    addr.sin_port = htons(port);
  }

  /** Connects to an ip and port **/
  void connect_with(String *ip, int port) {
    struct sockaddr_in other;

    // Create a new socket to interact with the other node.
    assert((curr = socket(AF_INET, SOCK_STREAM, 0)) != 0);

    // Setup the register address
    format_addr_(other, ip, port);

    // Connect to the other node
    assert(connect(curr, (struct sockaddr *)&other, sizeof(other)) >= 0);

    // Add it to the socket map
    socket_map.put(ip, port, curr);
  }

  /** Sends a register message to the given ip and port. **/
  void register_with(const char *ip, int port) {
    struct sockaddr_in reg;

    // Create a new socket to interact with the registrar.
    assert((curr = socket(AF_INET, SOCK_STREAM, 0)) != 0);

    // Setup the register address
    String s(ip);
    format_addr_(reg, &s, port);

    // Connect to the server
    assert(connect(curr, (struct sockaddr *)&reg, sizeof(reg)) >= 0);
    socket_map.put(s.clone(), port, curr);

    // Send them a message about our information.
    Register r;
    r.init(file_descriptor, curr, cur_message_id++);
    r.port = port_;
    memcpy(&r.client, &address, sizeof(address));
    assert(r.num_bytes() <= MSG_SIZE);

    char message[MSG_SIZE] = {0};
    char *buf = reinterpret_cast<char *>(message);
    r.encode(buf);

    // Send the registration message.
    send(curr, message, MSG_SIZE, 0);

    // Wait for the directory message back.
    read(curr, message, MSG_SIZE);

    char *read = reinterpret_cast<char *>(message);
    handle_msg(message_from(read));

    // Afterwards, say hello to the next door neighbor.
    hello_neighbor_();
  }

  /** Tells the node to our right hello. **/
  void hello_neighbor_() {
    pln("Hello Neighbor!");
    Status *status = new Status();
    status->init(id_, (id_ + 1) % num_nodes, cur_message_id++);
    StrBuff sb;
    status->set(sb.c("Hello from ").c(id_).c("!").get());
    send_msg(status);
  }

  /** Accept incoming connections from the given number of nodes. **/
  void accept_connections(size_t num_nodes) {
    assert(num_nodes >= 1);
    size_t n_waitfor = num_nodes - 1;
    ac.set(id_, file_descriptor, &curr, n_waitfor, &socket_map, &inbound,
           &lock);
    ac.start();
  }

  /** Waits for incoming connections and services them. Places block. **/
  void service() {
    while (true) {
      if (inbound.len() != 0) {
        handle_msg(inbound.pop());
      }
      if (outbound.len() != 0) {
        send_msg(outbound.pop());
      }
    }
  }

  /** Handles reception of a message. **/
  void handle_msg(Message *m) {
    assert(m != nullptr);
    switch (m->kind_) {
    case MsgKind::Register:
      handle_register(dynamic_cast<Register *>(m));
      break;
    case MsgKind::Directory:
      handle_directory(dynamic_cast<Directory *>(m));
      break;
    case MsgKind::Status:
      handle_status(dynamic_cast<Status *>(m));
      break;
    case MsgKind::Kill:
      pln("Shutting down gracefully...");
      exit(0);
    default:
      break;
    }
  }

  /** Broadcasts the message to all active sockets.  **/
  void broadcast_msg(Message *m) {
    assert(m != nullptr);
    for (size_t i = 0; i < dir.clients; i++) {
      if (i == (size_t)id_) {
        continue;
      }

      m->init(id_, i, cur_message_id++);
      send_msg(m);
    }
  }

  /** Sends the message to the id of the recipient **/
  void send_msg(Message *m) {
    assert(m != nullptr);
    char message[MSG_SIZE] = {0};
    char *buf = reinterpret_cast<char *>(message);
    m->encode(buf);
    send(get_socket(m->target_), message, MSG_SIZE, 0);
  }

  /** Handle a registration. The method assumes "this" is the registrar. **/
  void handle_register(Register *r) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(r->client.sin_addr), ip, INET_ADDRSTRLEN);

    String *sip = new String(ip);
    dir.add(sip, r->port);
    socket_map.put(sip, r->port, r->id_);
    p("Node Registered: ");
    print_node_info_(dir.clients - 1);

    if (dir.clients == num_nodes && id_ == 0) {
      pln("All nodes are connected");
      broadcast_msg(&dir);
    }
  }

  /** Gets the socket to write to given a node's id. **/
  int get_socket(size_t id) {
    assert(id < dir.clients);

    String *ip = dir.addresses.get(id);
    int port = dir.ports.get(id);

    assert(socket_map.contains_key(ip, port));

    Socket *sock = socket_map.get(ip, port);
    return sock->get();
  }

  /** Print node info from a node's id. **/
  void print_node_info_(size_t id) {
    String *ip = dir.addresses.get(id);
    int port = dir.ports.get(id);

    p("Id: ").p(id);
    p("\tAddress: ").p(ip->c_str()).p(":").p(port);
    p("\t@").pln(get_socket(id));
  }

  /** Handle a directory. The method assumes "this" is a client. **/
  void handle_directory(Directory *d) {
    dir.override(*d); // Set our directory to the result of the registration.
    num_nodes = dir.clients;
    accept_connections(num_nodes);

    pln("Finished registration with server. Found the following nodes:");
    for (size_t i = 0; i < dir.clients; i++) {
      String *ip = dir.addresses.get(i);
      int port = dir.ports.get(i);
      if (ip->equals(ip_) && port == port_) {
        id_ = i;
        continue;
      }
      if (i != 0) {
        connect_with(ip, port);
      }
      print_node_info_(i);
    }
  }

  /** Handle a status method. **/
  void handle_status(Status *s) {
    p("Received status from Id: ");
    p(s->sender_);
    p("\tMessage: ");
    pln(s->msg->c_str());
  }
};
