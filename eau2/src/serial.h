// lang: CwC
#pragma once

#include <arpa/inet.h>

#include "array.h"
#include "pack.h"

/** Represents the types of messages a node can send over the network. **/
enum class MsgKind { Status, Kill, Register, Directory };

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
    clear();  // Clear the array and push back all the doubles from bytes.
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
    clear();  // Clear the array and push back all the doubles from bytes.
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

  Register() { kind_ = MsgKind::Register; }

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

/** Represents a status upodate.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Status : public Message {
 public:
  String *msg;
  Status() { kind_ = MsgKind::Status; }
  void set(String *s) { msg = s; }

  void encode(unsigned char *&bytes) {
    Message::encode(bytes);
    packs(bytes, msg);
  }

  size_t num_bytes() {
    return Message::num_bytes() + sizeof(size_t) + msg->size() + 1;
  }

  void decode(unsigned char *&bytes) {
    Message::decode(bytes);
    msg = unpacks(bytes);
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

  void add(String *address, int port) {
    addresses.push_back(address);
    ports.push_back(port);
    clients++;
  }

  void override(Directory &dir) {
    for (size_t i = 0; i < clients; i++) {
      delete addresses.get(i);
    }
    addresses.clear();
    ports.clear();

    for (size_t i = 0; i < dir.clients; i++) {
      addresses.push_back(dir.addresses.get(i)->clone());
      ports.push_back(dir.ports.get(i));
    }
    clients = dir.clients;
  }
};

/** Unpack a message from a buffer **/
Message *message_from(unsigned char *&bytes) {
  unsigned char *type_buf = bytes;
  MsgKind k = static_cast<MsgKind>(unpackst(type_buf));
  Message *m;
  switch (k) {
    case MsgKind::Register:
      m = new Register();
      m->decode(bytes);
      return m;
    case MsgKind::Directory:
      m = new Directory();
      m->decode(bytes);
      return m;
    case MsgKind::Status:
      m = new Status();
      m->decode(bytes);
      return m;
    default:
      break;
  }
  return nullptr;
}
