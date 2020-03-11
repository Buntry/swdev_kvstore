// lang::CwC
#pragma once

#include <cstdlib>

#include "object.h"
#include "string.h"

/**
 * Queue: Represents a FIFO data structure.
 *        Capable of holding any Object in the given order.
 *
 * mutable
 *
 * authors: dermer.s@husky.neu.edu && Sarialli.n@northeastern.edu
 * implementors: griep.p@husky.neu.edu && colabella.a@husky.neu.edu
 */
class Queue : public Object {
 public:
  size_t size_;
  size_t num_elements_;
  size_t start_pos_;
  Object **elements_;

  // The Constructor
  Queue() {
    size_ = 0;
    num_elements_ = 0;
    start_pos_ = 0;
    elements_ = nullptr;
  }

  // The Destructor
  virtual ~Queue() { delete[] elements_; }

  // Iterates an index within the queue
  size_t offset_(size_t offset) { return (offset + start_pos_) % size_; }

  // Resizes the given queue to the new size,
  // preserving elements that were most recently added.
  void resize_to_(size_t new_size) {
    if (size_ == new_size) return;

    Object **new_elements_ = new Object *[new_size];
    size_t num_deleted = 0;
    for (size_t i = 0; i < num_elements_; i++) {
      if (i < new_size) {
        new_elements_[i] = elements_[offset_(i)];
      } else {
        num_deleted++;
      }
    }
    start_pos_ = 0;
    num_elements_ -= num_deleted;
    size_ = new_size;
    delete elements_;
    elements_ = new_elements_;
  }

  // Grows this queue to fit the given number of elements.
  void grow_to_fit_(size_t num_elements) {
    if (num_elements > size_) {
      resize_to_(num_elements * 2);
    }
  }

  // adds the given object to the beginning of this queue.
  // Does not take ownership of o
  // undefined behavior: the same pointer is pushed more than once or o is this
  virtual void push(Object *o) {
    grow_to_fit_(num_elements_ + 1);
    elements_[offset_(num_elements_)] = o;
    num_elements_++;
  }

  // removes and returns the object at the end of this queue.
  // errors if there is nothing in the queue
  virtual Object *pop() {
    Object *old = elements_[start_pos_];
    elements_[start_pos_] = nullptr;
    start_pos_ = offset_(1);
    num_elements_--;
    return old;
  }

  // returns the object at the end of this queue but does not remove it.
  // errors if there is nothing in the queue
  virtual Object *peek() {
    if (num_elements_ == 0) exit(1);
    return elements_[start_pos_];
  }

  // gives the total number of elements in this Queue
  virtual size_t len() { return num_elements_; }

  // true if o is a queue and all of the elements of this queue,
  // in the same order, are equal to o's
  virtual bool equals(Object *other) {
    Queue *q = dynamic_cast<Queue *>(other);
    if (q == nullptr) return false;
    if (q->len() != this->len()) return false;

    for (size_t i = 0; i < len(); i++) {
      if (!q->elements_[offset_(i)]->equals(this->elements_[offset_(i)])) {
        return false;
      }
    }
    return true;
  }

  // hash of this queue
  virtual size_t hash() {
    size_t hash = 0;
    for (size_t i = 0; i < len(); i++) {
      hash ^= elements_[offset_(i)]->hash();
    }
    return hash;
  }
};

/**
 * StringQueue: Represents a FIFO data structure.
 *        Capable of holding any String in the given order.
 *
 * mutable
 *
 * authors: dermer.s@husky.neu.edu && Sarialli.n@northeastern.edu
 * implementors: griep.p@husky.neu.edu && colabella.a@husky.neu.edu
 */
class StringQueue : public Queue {
 public:
  String *peek() { return dynamic_cast<String *>(Queue::peek()); }
  String *pop() { return dynamic_cast<String *>(Queue::pop()); }
};
