// lang: CwC
#pragma once

#include <arpa/inet.h>

#include "../store/column.h"
#include "../store/kv.h"
#include "../utils/queue.h"
#include "../utils/thread.h"

/** Represents the types of messages a node can send over the network. **/
enum class MsgKind { Status, Register, Directory, Kill, Get, Put, Reply };

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

  virtual void serialize(Serializer &ser) {
    Message::serialize(ser);
    ser.write(msg_);
  }

  virtual Status *deserialize(Deserializer &dser) {
    Message::deserialize(dser);
    msg_ = String::deserialize(dser);
    return this;
  }
};

/** Represents a message to terminate the program.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Kill : public Message {
public:
  Kill() { kind_ = MsgKind::Kill; }
};

/** Represents a request for the value associated with the given key.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Get : public Message {
public:
  Key *key_ = nullptr;

  Get() { kind_ = MsgKind::Get; }
  Get(Key *key) : Get() { key_ = key; }
  ~Get() { delete key_; }
  void set(Key *key) { key_ = key; }

  Key *key() { return key_; }

  void serialize(Serializer &ser) {
    Message::serialize(ser);
    key_->serialize(ser);
  }

  Get *deserialize(Deserializer &dser) {
    Message::deserialize(dser);
    key_ = Key::deserialize(dser);
    return this;
  }
};

/** Represents a request to place the key and value pair at the given node.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Put : public Message {
public:
  Key *key_ = nullptr;
  Value *value_ = nullptr;

  Put() { kind_ = MsgKind::Put; }
  Put(Key *key, Value *value) : Put() {
    key_ = key;
    value_ = value;
  }
  ~Put() {
    delete key_;
    delete value_;
  }

  void set(Key *key, Value *value) {
    key_ = key;
    value_ = value;
  }

  Key *key() { return key_; }
  Value *value() { return value_; }

  void serialize(Serializer &ser) {
    Message::serialize(ser);
    key_->serialize(ser);
    value_->serialize(ser);
  }

  Put *deserialize(Deserializer &dser) {
    Message::deserialize(dser);
    key_ = Key::deserialize(dser);
    value_ = Value::deserialize(dser);
    return this;
  }
};

/** Represents a response to a get request with the corresponding data.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Reply : public Message {
public:
  Key *key_ = nullptr;
  Value *value_ = nullptr;

  Reply() { kind_ = MsgKind::Reply; }
  Reply(Key *key, Value *value) : Reply() {
    key_ = key;
    value_ = value;
  }
  ~Reply() {
    delete key_;
    delete value_;
  }

  void set(Key *key, Value *value) {
    key_ = key;
    value_ = value;
  }

  Key *key() { return key_; }
  Value *value() { return value_; }

  void serialize(Serializer &ser) {
    Message::serialize(ser);
    key_->serialize(ser);
    value_->serialize(ser);
  }

  Reply *deserialize(Deserializer &dser) {
    Message::deserialize(dser);
    key_ = Key::deserialize(dser);
    value_ = Value::deserialize(dser);
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
  case MsgKind::Kill:
    msg = new Kill();
    break;
  case MsgKind::Get:
    msg = new Get();
    break;
  case MsgKind::Put:
    msg = new Put();
    break;
  case MsgKind::Reply:
    msg = new Reply();
    break;
  default:
    assert(false);
  }
  msg->deserialize(dser);
  return msg;
}

generate_object_classqueue(MessageQueue, Message);

/** Represents a Message Queue that is concurrent. **/
class ConcurrentMessageQueue : public MessageQueue {
public:
  Lock lock_;

  /** Pushes a message while locking the queue. **/
  void push(Message *m) {
    lock_.lock();
    MessageQueue::push(m);
    lock_.notify_all();
    lock_.unlock();
  }

  /** Pops a message while locking the queue. **/
  Message *pop() {
    lock_.lock();
    while (size() == 0) {
      lock_.wait();
    }
    Message *hold = MessageQueue::pop();
    lock_.unlock();
    return hold;
  }
};