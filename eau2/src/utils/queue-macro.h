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
    assert(size() > 0);
    Object *hold = get(0);
    start_pos_ = offset_(start_pos_ + 1);
    num_elements_--;
    return hold;
  }

  virtual Object *peek() {
    assert(size() > 0);
    return get(0);
  }

  // gives the total number of elements in this Queue
  virtual size_t len() { return size(); }
};

/**
 * StringQueue: Represents a FIFO data structure.
 *        Capable of holding any String in the given order.
 *
 * implementors: griep.p@husky.neu.edu && colabella.a@husky.neu.edu
 */
// class StringQueue : public Queue {
//  public:
//   String *peek() { return dynamic_cast<String *>(Queue::peek()); }
//   String *pop() { return dynamic_cast<String *>(Queue::pop()); }
// };

#define generate_object_classqueue(KlassQueue, SubObj)                       \
  class KlassQueue : public Queue {                                          \
   public:                                                                   \
    virtual SubObj *peek() { return dynamic_cast<SubObj *>(Queue::peek()); } \
    virtual SubObj *pop() { return dynamic_cast<SubObj *>(Queue::pop()); }   \
  }

generate_object_classqueue(StringQueue, String);
