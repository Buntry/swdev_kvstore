// lang: CwC
#pragma once

#include <arpa/inet.h>

#include "../utils/array.h"
#include "../utils/pack.h"

/** Represents the types of messages a node can send over the network. **/
enum class MsgKind {
  Ack,
  Nack,
  Put,
  Reply,
  Get,
  WaitAndGet,
  Status,
  Kill,
  Register,
  Directory
};

/** Represents a String Array that is serializable.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class SerializableStringArray : public StringArray {
public:
  void encode(unsigned char *&bytes) {
    packst(bytes, size());
    for (size_t i = 0; i < size(); i++) {
      packs(bytes, get(i));
    }
  }

  size_t num_bytes() {
    size_t byte_count = sizeof(size_t);
    for (size_t i = 0; i < size(); i++) {
      String *s = get(i);
      if (s == nullptr) {
        byte_count++;
      } else {
        byte_count += sizeof(size_t) + s->size() + 1;
      }
    }
    return byte_count;
  }

  void decode(unsigned char *&bytes) {
    clear();
    size_t len = unpackst(bytes);
    for (size_t i = 0; i < len; i++) {
      push_back(unpacks(bytes));
    }
  }
};

/** Represents a IntArray that is serializable.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class SerializableIntArray : public IntArray {
public:
  void encode(unsigned char *&bytes) {
    packst(bytes, size());
    for (size_t i = 0; i < size(); i++) {
      packi(bytes, get(i));
    }
  }

  size_t num_bytes() { return sizeof(size_t) + (sizeof(int) * size()); }

  void decode(unsigned char *&bytes) {
    clear(); // Clear the array and push back all the doubles from bytes.
    size_t len = unpackst(bytes);
    for (size_t i = 0; i < len; i++) {
      push_back(unpacki(bytes));
    }
  }
};

/** Represents a DoubleArray that is serializable.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class SerializableDoubleArray : public DoubleArray {
public:
  void encode(unsigned char *&bytes) {
    packst(bytes, size());
    for (size_t i = 0; i < size(); i++) {
      packd(bytes, get(i));
    }
  }

  size_t num_bytes() { return sizeof(size_t) + (sizeof(double) * size()); }

  void decode(unsigned char *&bytes) {
    clear(); // Clear the array and push back all the doubles from bytes.
    size_t len = unpackst(bytes);
    for (size_t i = 0; i < len; i++) {
      push_back(unpackd(bytes));
    }
  }
};

/** Represents a message.
 *  @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Message : public Object {
public:
  MsgKind kind_;
  size_t sender_;
  size_t target_;
  size_t id_;

  void init(size_t sender, size_t target, size_t id) {
    sender_ = sender;
    target_ = target;
    id_ = id;
  }

  virtual void encode(unsigned char *&bytes) {
    packst(bytes, static_cast<size_t>(kind_));
    packst(bytes, sender_);
    packst(bytes, target_);
    packst(bytes, id_);
  }

  virtual void decode(unsigned char *&bytes) {
    kind_ = static_cast<MsgKind>(unpackst(bytes));
    sender_ = unpackst(bytes);
    target_ = unpackst(bytes);
    id_ = unpackst(bytes);
  }

  virtual size_t num_bytes() { return sizeof(size_t) * 4; }
};

/** Represents all the information needed to register.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Register : public Message {
public:
  sockaddr_in client;
  size_t port;

  Register() {
    kind_ = MsgKind::Register;
    port = 0;
  }

  void set(sockaddr_in c, size_t p) {
    client = c;
    port = p;
  }

  void encode(unsigned char *&bytes) {
    Message::encode(bytes);
    packst(bytes, port);
    memcpy(bytes, &client.sin_addr, sizeof(client.sin_addr));
    advance(bytes, sizeof(client.sin_addr));
  }

  size_t num_bytes() {
    return Message::num_bytes() + sizeof(size_t) + sizeof(client.sin_addr);
  }

  void decode(unsigned char *&bytes) {
    Message::decode(bytes);
    port = unpackst(bytes);

    client.sin_family = AF_INET;
    memcpy(&client.sin_addr, bytes, sizeof(client.sin_addr));
    advance(bytes, sizeof(client.sin_addr));
    client.sin_port = htons(port);
  }
};

/** Represents a Directory of node addresses. A Directory owns the strings
 * inside of it.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Directory : public Message {
public:
  SerializableStringArray addresses;
  SerializableIntArray ports;
  size_t clients;

  Directory() {
    kind_ = MsgKind::Directory;
    clients = 0;
  }

  ~Directory() {
    for (size_t i = 0; i < clients; i++) {
      delete addresses.get(i);
    }
  }

  void add(String *address, int port) {
    addresses.push_back(address);
    ports.push_back(port);
    clients++;
  }

  void encode(unsigned char *&bytes) {
    Message::encode(bytes);
    packst(bytes, clients);
    addresses.encode(bytes);
    ports.encode(bytes);
  }

  size_t num_bytes() {
    return Message::num_bytes() + sizeof(size_t) + addresses.num_bytes() +
           ports.num_bytes();
  }

  void decode(unsigned char *&bytes) {
    Message::decode(bytes);
    clients = unpackst(bytes);
    addresses.decode(bytes);
    ports.decode(bytes);
  }
};

/** Represents an Ack: an acknowledgement Message
 * that a message was properly received.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Ack : public Message {
public:
  Ack() { kind_ = MsgKind::Ack; }
};

/** Represents an Nack: a message that indicates
 * that an expected message was never received.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Nack : public Message {
public:
  Nack() { kind_ = MsgKind::Nack; }
};

/** Represents a Put: a message to be sent
 * to another client.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Put : public Message {
public:
  String *msg_;

  Put() {
    kind_ = MsgKind::Put;
    msg_ = nullptr;
  }

  ~Put() { delete msg_; }

  void set(String *msg) { msg_ = msg; }

  void encode(unsigned char *&bytes) {
    Message::encode(bytes);
    packs(bytes, msg_);
  }

  size_t num_bytes() {
    size_t num = Message::num_bytes();
    if (msg_ == nullptr) {
      num++;
    } else {
      num += sizeof(size_t) + msg_->size() + 1;
    }

    return num;
  }

  void decode(unsigned char *&bytes) {
    Message::decode(bytes);
    msg_ = unpacks(bytes);
  }
};

/** Represents a Reply: a message that is sent
 * in response to a message received from a client.
 * @author: griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Reply : public Message {
public:
  String *msg_;

  Reply() {
    kind_ = MsgKind::Reply;
    msg_ = nullptr;
  }

  ~Reply() { delete msg_; }

  void set(String *msg) { msg_ = msg; }

  void encode(unsigned char *&bytes) {
    Message::encode(bytes);
    packs(bytes, msg_);
  }

  size_t num_bytes() {
    size_t num = Message::num_bytes();
    if (msg_ == nullptr) {
      num++;
    } else {
      num += sizeof(size_t) + msg_->size() + 1;
    }

    return num;
  }

  void decode(unsigned char *&bytes) {
    Message::decode(bytes);
    msg_ = unpacks(bytes);
  }
};

/** Represents a Get: a message that is sent
 * and indicates a request to receive a message from a
 * specific client
 * @author: griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Get : public Message {
public:
  Get() { kind_ = MsgKind::Get; }
};

/** Represents a WaitAndGet: a message that is sent
 * that indicates it wishes to receive a message from a
 * client after a specified wait period (in ms)..
 * @author: griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class WaitAndGet : public Message {
public:
  size_t wait_ms_;

  WaitAndGet() {
    kind_ = MsgKind::WaitAndGet;
    wait_ms_ = 0;
  }

  void set(size_t wait_ms) { wait_ms_ = wait_ms; }

  void encode(unsigned char *&bytes) {
    Message::encode(bytes);
    packst(bytes, wait_ms_);
  }

  size_t num_bytes() { return Message::num_bytes() + sizeof(size_t); }

  void decode(unsigned char *&bytes) {
    Message::decode(bytes);
    wait_ms_ = unpackst(bytes);
  }
};

/** Represents a Status: a message that a client
 * emits to the Server.
 * @author: griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Status : public Message {
public:
  String *msg_; // Owned

  Status() {
    kind_ = MsgKind::Status;
    msg_ = nullptr;
  }

  ~Status() { delete msg_; }

  void set(String *msg) { msg_ = msg; }

  void encode(unsigned char *&bytes) {
    Message::encode(bytes);
    packs(bytes, msg_);
  }

  size_t num_bytes() {
    size_t num = Message::num_bytes();
    if (msg_ == nullptr) {
      num++;
    } else {
      num += sizeof(size_t) + msg_->size() + 1;
    }

    return num;
  }

  void decode(unsigned char *&bytes) {
    Message::decode(bytes);
    msg_ = unpacks(bytes);
  }
};

/** Represents a Kill: a message that indicates
 * a request to stop the Node and exit.
 * @author: griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Kill : public Message {
public:
  Kill() { kind_ = MsgKind::Kill; }
};
