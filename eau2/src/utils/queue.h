// lang::CwC
#pragma once

#include <cstdlib>

#include "array.h"

/**
 * Queue: Represents a FIFO data structure.
 *        Capable of holding any Object in the given order.
 *
 * implementors: griep.p@husky.neu.edu && colabella.a@husky.neu.edu
 */
class Queue : public Array {
public:
  virtual void push(Object *o) { push_back(o); }
  virtual Object *pop() {
    Object *hold = set(0, nullptr);
    start_pos_ = offset_(1);
    num_elements_--;
    return hold;
  }

  virtual Object *peek() { return get(0); }
};

#define generate_object_classqueue(KlassQueue, SubObj)                         \
  class KlassQueue : public Queue {                                            \
  public:                                                                      \
    virtual SubObj *peek() { return dynamic_cast<SubObj *>(Queue::peek()); }   \
    virtual SubObj *pop() { return dynamic_cast<SubObj *>(Queue::pop()); }     \
  }

/**
 * StringQueue: Represents a FIFO data structure.
 *        Capable of holding any String in the given order.
 *
 * implementors: griep.p@husky.neu.edu && colabella.a@husky.neu.edu
 */
generate_object_classqueue(StringQueue, String);
