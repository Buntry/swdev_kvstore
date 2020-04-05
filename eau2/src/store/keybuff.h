#pragma once
// lang: CwC

#include "kvstore-fd.h"

/** Represents a way to easily create keys.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class KeyBuff : public Object {
public:
  StrBuff buf_;
  size_t node_;

  KeyBuff(Key *orig) {
    buf_.c(orig->key()->c_str());
    node_ = orig->node();
  }

  KeyBuff &c(String &s) {
    buf_.c(s);
    return *this;
  }
  KeyBuff &c(size_t v) {
    buf_.c(v);
    return *this;
  }
  KeyBuff &c(const char *v) {
    buf_.c(v);
    return *this;
  }

  KeyBuff &set_node(size_t node) {
    node_ = node;
    return *this;
  }

  Key *get() {
    String *s = buf_.get();
    Key *k = new Key(s->steal(), node_);
    delete s;
    return k;
  }
};