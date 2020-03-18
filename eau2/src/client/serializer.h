// lang: CwC
#pragma once

#include "../store/dataframe.h"
#include "message.h"

size_t MAX_BUF_SIZE = 1024; // The maximum size of a buffer.

/** Represents the types of messages a node can send over the network. **/
enum class ValKind { String, Chunk, DataFrame };

/** Represents a Serializer that can take a Message and
 * serialize its contents.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Serializer : public Object {
public:
  char *serialize(Message *m) {
    assert(m != nullptr);
    char *buf = new char[MAX_BUF_SIZE];
    m->encode(buf);
    return buf;
  }
};

/** Represents a Deserializer that can take a Message and its ValKind and
 * Deserialize its contents.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Deserializer : public Object {
public:
  char *deserialize(Message *m, ValKind v) {
    assert(m != nullptr);
    char *buf = new char[MAX_BUF_SIZE];
    m->decode(buf);

    switch (v) {
    case ValKind::String:
      
      break;
    case ValKind::Chunk:

      break;
    case ValKind::DataFrame:

      break;
    default:
      break;
    }
    return buf;
  }
};
