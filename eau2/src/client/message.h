// lang: CwC
#pragma once

#include <arpa/inet.h>

#include "../store/column.h"

/** Represents the types of messages a node can send over the network. **/
enum class MsgKind { Status, Register, Directory };

/** Represents a message.
 *  @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Message : public Object {
public:
  MsgKind kind_;
  size_t sender_;
  size_t target_;
  size_t id_;

  /** Easy way to initialize the sender, target, and id of a msg. **/
  void init(size_t sender, size_t target, size_t id) {
    sender_ = sender;
    target_ = target;
    id_ = id;
  }

  MsgKind kind() { return kind_; }
  size_t sender() { return sender_; }
  size_t target() { return target_; }

  virtual void serialize(Serializer &ser) {
    ser.write(static_cast<size_t>(kind_));
    ser.write(sender_);
    ser.write(target_);
    ser.write(id_);
  }

  virtual Message *deserialize(Deserializer &dser) {
    kind_ = static_cast<MsgKind>(dser.read_size_t());
    sender_ = dser.read_size_t();
    target_ = dser.read_size_t();
    id_ = dser.read_size_t();
    return this;
  }

  static Message *from(Deserializer &dser);
};

/** Represents all the information needed to register.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Register : public Message {
public:
  sockaddr_in address_;
  size_t port_;

  Register() { kind_ = MsgKind::Register; }

  void set(sockaddr_in address, size_t port) {
    address_ = address;
    port_ = port;
  }

  void serialize(Serializer &ser) {
    Message::serialize(ser);
    ser.write(port_);
    String *ip = new String(inet_ntoa(address_.sin_addr));
    ser.write(ip);
    delete ip;
  }

  Register *deserialize(Deserializer &dser) {
    Message::deserialize(dser);
    port_ = dser.read_size_t();
    String *ip = String::deserialize(dser);

    address_.sin_family = AF_INET;
    inet_pton(AF_INET, ip->c_str(), &address_.sin_addr);
    address_.sin_port = htons(port_);

    delete ip;
    return this;
  }
};

/** Represents a Directory of node addresses. A Directory owns the strings
 * inside of it.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Directory : public Message {
public:
  StringColumn *addresses_ = nullptr;
  IntColumn *ports_ = nullptr;
  size_t clients_;

  Directory() {
    kind_ = MsgKind::Directory;
    addresses_ = new StringColumn();
    ports_ = new IntColumn();
    clients_ = 0;
  }

  ~Directory() {
    delete addresses_;
    delete ports_;
  }

  size_t clients() { return clients_; }
  String *address(size_t idx) { return addresses_->get(idx); }
  size_t port(size_t idx) { return ports_->get(idx); }

  /** Adds a client to the directory **/
  void add_client(String &address, int port) {
    addresses_->push_back(&address);
    ports_->push_back(port);
    clients_++;
  }

  void serialize(Serializer &ser) {
    Message::serialize(ser);
    ser.write(clients_);
    ser.write(addresses_);
    ser.write(ports_);
  }

  Directory *deserialize(Deserializer &dser) {
    Message::deserialize(dser);

    delete addresses_;
    delete ports_;

    clients_ = dser.read_size_t();
    addresses_ = Column::deserialize(dser)->as_string();
    ports_ = Column::deserialize(dser)->as_int();
    return this;
  }
};

/** Represents a Status message with one string inside of it.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Status : public Message {
public:
  String *msg_ = nullptr;

  Status() {}
  Status(String *msg) : msg_(msg) { kind_ = MsgKind::Status; }
  ~Status() { delete msg_; }
  String *s() { return msg_; }

  virtual void serialize(Serializer &ser) { ser.write(msg_); }

  virtual Status *deserialize(Deserializer &dser) {
    msg_ = String::deserialize(dser);
    return this;
  }
};

/** Acquires a message from a deserializer object. **/
Message *Message::from(Deserializer &dser) {
  MsgKind kind = static_cast<MsgKind>(dser.peek_size_t());
  Message *msg;
  switch (kind) {
  case MsgKind::Register:
    msg = new Register();
    break;
  case MsgKind::Directory:
    msg = new Directory();
    break;
  case MsgKind::Status:
    msg = new Status();
    break;
  default:
    msg = nullptr;
    break;
  }
  msg->deserialize(dser);
  return msg;
}

// /** Represents an Ack: an acknowledgement Message
//  * that a message was properly received.
//  * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
// class Ack : public Message {
// public:
//   Ack() { kind_ = MsgKind::Ack; }
// };

// /** Represents an Nack: a message that indicates
//  * that an expected message was never received.
//  * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
// class Nack : public Message {
// public:
//   Nack() { kind_ = MsgKind::Nack; }
// };

// /** Represents a Put: a message to be sent
//  * to another client.
//  * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
// class Put : public Message {
// public:
//   String *msg_;

//   Put() {
//     kind_ = MsgKind::Put;
//     msg_ = nullptr;
//   }

//   ~Put() { delete msg_; }

//   void set(String *msg) { msg_ = msg; }

//   void encode(char *&bytes) {
//     Message::encode(bytes);
//     packs(bytes, msg_);
//   }

//   size_t num_bytes() {
//     size_t num = Message::num_bytes();
//     if (msg_ == nullptr) {
//       num++;
//     } else {
//       num += sizeof(size_t) + msg_->size() + 1;
//     }

//     return num;
//   }

//   void decode(char *&bytes) {
//     Message::decode(bytes);
//     msg_ = unpacks(bytes);
//   }

//   void serialize(Serializer &ser) {
//     Message::serialize(ser);
//     ser.write(msg_->c_str(), msg_->size());
//   }

//   virtual Put *deserialize(Deserializer &dser) {
//     Put *p = new Put();
//     p->sender_ = dser.read_size_t();
//     p->target_ = dser.read_size_t();
//     p->id_ = dser.read_size_t();
//     p->msg_ = String::deserialize(dser);

//     return p;
//   }
// };

// /** Represents a Reply: a message that is sent
//  * in response to a message received from a client.
//  * @author: griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
// class Reply : public Message {
// public:
//   String *msg_;

//   Reply() {
//     kind_ = MsgKind::Reply;
//     msg_ = nullptr;
//   }

//   ~Reply() { delete msg_; }

//   void set(String *msg) { msg_ = msg; }

//   void encode(char *&bytes) {
//     Message::encode(bytes);
//     packs(bytes, msg_);
//   }

//   size_t num_bytes() {
//     size_t num = Message::num_bytes();
//     if (msg_ == nullptr) {
//       num++;
//     } else {
//       num += sizeof(size_t) + msg_->size() + 1;
//     }

//     return num;
//   }

//   void decode(char *&bytes) {
//     Message::decode(bytes);
//     msg_ = unpacks(bytes);
//   }

//   void serialize(Serializer &ser) {
//     Message::serialize(ser);
//     ser.write(msg_->c_str(), msg_->size());
//   }

//   virtual Reply *deserialize(Deserializer &dser) {
//     Reply *r = new Reply();
//     r->sender_ = dser.read_size_t();
//     r->target_ = dser.read_size_t();
//     r->id_ = dser.read_size_t();
//     r->msg_ = String::deserialize(dser);

//     return r;
//   }
// };

// /** Represents a Get: a message that is sent
//  * and indicates a request to receive a message from a
//  * specific client
//  * @author: griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
// class Get : public Message {
// public:
//   Get() { kind_ = MsgKind::Get; }
// };

// /** Represents a WaitAndGet: a message that is sent
//  * that indicates it wishes to receive a message from a
//  * client after a specified wait period (in ms)..
//  * @author: griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
// class WaitAndGet : public Message {
// public:
//   size_t wait_ms_;

//   WaitAndGet() {
//     kind_ = MsgKind::WaitAndGet;
//     wait_ms_ = 0;
//   }

//   void set(size_t wait_ms) { wait_ms_ = wait_ms; }

//   void encode(char *&bytes) {
//     Message::encode(bytes);
//     packst(bytes, wait_ms_);
//   }

//   size_t num_bytes() { return Message::num_bytes() + sizeof(size_t); }

//   void decode(char *&bytes) {
//     Message::decode(bytes);
//     wait_ms_ = unpackst(bytes);
//   }

//   void serialize(Serializer &ser) {
//     Message::serialize(ser);
//     ser.write(wait_ms_);
//   }

//   virtual WaitAndGet *deserialize(Deserializer &dser) {
//     WaitAndGet *w = new WaitAndGet();
//     w->sender_ = dser.read_size_t();
//     w->target_ = dser.read_size_t();
//     w->id_ = dser.read_size_t();
//     w->wait_ms_ = dser.read_size_t();

//     return w;
//   }
// };

// /** Represents a Status: a message that a client
//  * emits to the Server.
//  * @author: griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
// class Status : public Message {
// public:
//   String *msg_; // Owned

//   Status() {
//     kind_ = MsgKind::Status;
//     msg_ = nullptr;
//   }

//   ~Status() { delete msg_; }

//   void set(String *msg) { msg_ = msg; }

//   void encode(char *&bytes) {
//     Message::encode(bytes);
//     packs(bytes, msg_);
//   }

//   size_t num_bytes() {
//     size_t num = Message::num_bytes();
//     if (msg_ == nullptr) {
//       num++;
//     } else {
//       num += sizeof(size_t) + msg_->size() + 1;
//     }

//     return num;
//   }

//   void decode(char *&bytes) {
//     Message::decode(bytes);
//     msg_ = unpacks(bytes);
//   }

//   void serialize(Serializer &ser) {
//     Message::serialize(ser);
//     ser.write(msg_->c_str(), msg_->size());
//   }

//   virtual Status *deserialize(Deserializer &dser) {
//     Status *s = new Status();
//     s->sender_ = dser.read_size_t();
//     s->target_ = dser.read_size_t();
//     s->id_ = dser.read_size_t();
//     s->msg_ = String::deserialize(dser);

//     return s;
//   }
// };

// /** Represents a Kill: a message that indicates
//  * a request to stop the Node and exit.
//  * @author: griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
// class Kill : public Message {
// public:
//   Kill() { kind_ = MsgKind::Kill; }
// };