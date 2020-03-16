// lang::CwC
#pragma once

#include <cstdlib>

#include "array.h"

/**
 * Queue: Represents a FIFO data structure.
 *        Capable of holding any Object in the given order.
 *
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu
 */
#define generate_classqueue(KlassQueue, KlassArray, Stores)                    \
  class KlassQueue : public KlassArray {                                       \
  public:                                                                      \
    virtual void push(Stores s) { push_back(s); }                              \
    virtual Stores pop() {                                                     \
      Stores hold = peek();                                                    \
      start_pos_ = offset_(1);                                                 \
      num_elements_--;                                                         \
      return hold;                                                             \
    }                                                                          \
    virtual Stores peek() { return get(0); }                                   \
  }

generate_classqueue(Queue, Array, Object *);  // Queue
generate_classqueue(IntQueue, IntArray, int); // IntQueue

#define generate_object_classqueue(KlassQueue, SubObj)                         \
  class KlassQueue : public Queue {                                            \
  public:                                                                      \
    virtual SubObj *peek() { return dynamic_cast<SubObj *>(Queue::peek()); }   \
    virtual SubObj *pop() { return dynamic_cast<SubObj *>(Queue::pop()); }     \
  }

generate_object_classqueue(StringQueue, String); // StringQueue
