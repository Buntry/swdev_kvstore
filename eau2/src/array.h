// lang: CwC
#pragma once
#include <assert.h>

#include "object.h"
#include "string.h"
#include "util.h"

static const size_t CHUNK_SIZE = 1024;

#define gen_array(KlassArray, Stores)                                          \
  class KlassArray : public Object {                                           \
  public:                                                                      \
    size_t num_chunks_;                                                        \
    size_t num_elements_;                                                      \
    size_t capacity_;                                                          \
    Stores **elements_;                                                        \
                                                                               \
    KlassArray() : Object() {                                                  \
      num_chunks_ = 1;                                                         \
      num_elements_ = 0;                                                       \
      capacity_ = CHUNK_SIZE;                                                  \
      elements_ = new Stores *[num_chunks_];                                   \
      elements_[0] = new Stores[capacity_];                                    \
    }                                                                          \
                                                                               \
    virtual ~KlassArray() {                                                    \
      for (size_t i = 0; i < num_chunks_; i++) {                               \
        delete[] elements_[i];                                                 \
      }                                                                        \
      delete[] elements_;                                                      \
    }                                                                          \
    void _resize_to_(size_t new_capacity) {                                    \
      if ((new_capacity / CHUNK_SIZE) <= num_chunks_) {                        \
        return;                                                                \
      }                                                                        \
      size_t new_num_chunks = (new_capacity / CHUNK_SIZE) + 1;                 \
      Stores **new_elements = new Stores *[new_num_chunks];                    \
      for (size_t i = 0; i < num_chunks_; i++) {                               \
        if (i < new_num_chunks) {                                              \
          new_elements[i] = elements_[i];                                      \
        } else {                                                               \
          delete[] elements_[i];                                               \
        }                                                                      \
      }                                                                        \
      for (size_t i = num_chunks_; i < new_num_chunks; i++) {                  \
        new_elements[i] = new Stores[CHUNK_SIZE];                              \
      }                                                                        \
      delete[] elements_;                                                      \
      elements_ = new_elements;                                                \
      capacity_ = CHUNK_SIZE * new_num_chunks;                                 \
      num_chunks_ = new_num_chunks;                                            \
      if (num_elements_ >= capacity_) {                                        \
        num_elements_ -= (num_elements_ - capacity_);                          \
      }                                                                        \
    }                                                                          \
                                                                               \
    void _grow_to_fit_(size_t num_elements) {                                  \
      if (num_elements > capacity_) {                                          \
        _resize_to_(num_elements * 2);                                         \
      }                                                                        \
    }                                                                          \
                                                                               \
    virtual void push_back(Stores e) { add(num_elements_, e); }                \
                                                                               \
    virtual void add(size_t index, Stores e) {                                 \
      _grow_to_fit_(num_elements_ + 1);                                        \
      for (size_t i = num_elements_; i > index; i--) {                         \
        elements_[_row(i)][_col(i)] = elements_[_row(i - 1)][_col(i - 1)];     \
      }                                                                        \
      elements_[_row(index)][_col(index)] = e;                                 \
      num_elements_++;                                                         \
    }                                                                          \
                                                                               \
    virtual void concat(KlassArray *c) { add_all(num_elements_, c); }          \
                                                                               \
    virtual void add_all(size_t index, KlassArray *c) {                        \
      size_t len = c->size();                                                  \
      size_t temp_num_elements = num_elements_;                                \
      _grow_to_fit_(num_elements_ + len);                                      \
      for (size_t i = num_elements_ + len; i > index + len; i--) {             \
        size_t shift = i - len;                                                \
        elements_[_row(i)][_col(i)] = elements_[_row(shift)][_col(shift)];     \
      }                                                                        \
      for (size_t i = index; i - index < len; i++) {                           \
        elements_[_row(i)][_col(i)] = c->get(i - index);                       \
        temp_num_elements++;                                                   \
      }                                                                        \
      num_elements_ = temp_num_elements;                                       \
    }                                                                          \
                                                                               \
    virtual void clear() {                                                     \
      _resize_to_(0);                                                          \
      num_elements_ = 0;                                                       \
    }                                                                          \
                                                                               \
    virtual bool equals(Object *o) {                                           \
      KlassArray *that = dynamic_cast<KlassArray *>(o);                        \
      if (that == nullptr)                                                     \
        return false;                                                          \
      if (that->size() != this->size())                                        \
        return false;                                                          \
      for (size_t i = 0; i < this->size(); i++) {                              \
        if (!Util::equals(this->get(i), that->get(i))) {                       \
          return false;                                                        \
        }                                                                      \
      }                                                                        \
      return true;                                                             \
    }                                                                          \
                                                                               \
    Stores get(size_t index) { return elements_[_row(index)][_col(index)]; }   \
    virtual size_t _row(size_t index) { return index / CHUNK_SIZE; }           \
    virtual size_t _col(size_t index) { return index % CHUNK_SIZE; }           \
                                                                               \
    size_t hash() {                                                            \
      size_t hash = 0;                                                         \
      for (size_t i = 0; i < size(); i++) {                                    \
        hash += Util::hash(get(i));                                            \
      }                                                                        \
      return hash;                                                             \
    }                                                                          \
                                                                               \
    virtual size_t index_of(Stores o) {                                        \
      for (size_t i = 0; i < size(); i++) {                                    \
        if (Util::equals(get(i), o)) {                                         \
          return i;                                                            \
        }                                                                      \
      }                                                                        \
      return size() + 1;                                                       \
    }                                                                          \
                                                                               \
    virtual Stores remove(size_t index) {                                      \
      Stores old = get(index);                                                 \
      for (size_t i = index; i < num_elements_; i++) {                         \
        elements_[_row(i)][_col(i)] = elements_[_row(i + 1)][_col(i + 1)];     \
      }                                                                        \
      num_elements_--;                                                         \
      return old;                                                              \
    }                                                                          \
                                                                               \
    virtual Stores set(size_t index, Stores e) {                               \
      Stores old = get(index);                                                 \
      elements_[_row(index)][_col(index)] = e;                                 \
      return old;                                                              \
    }                                                                          \
                                                                               \
    virtual size_t size() { return num_elements_; }                            \
  }

gen_array(Array, Object *);
gen_array(BoolArray, bool);
gen_array(IntArray, int);
gen_array(FloatArray, float);
gen_array(DoubleArray, double);

/**
 * @brief This is an Array class for Strings.
 * It has the same time and space complexity as Array.
 *
 * @author griep.p@husky.neu.edu, colabella.a@husky.neu.edu
 */
class StringArray : public Object {
public:
  size_t capacity_;
  size_t num_chunks_;
  size_t num_elements_;
  String ***elements_;

  // Constructor for StringArray
  StringArray() : Object() {
    num_chunks_ = 1;
    num_elements_ = 0;
    capacity_ = CHUNK_SIZE;
    elements_ = new String **[num_chunks_];
    elements_[0] = new String *[CHUNK_SIZE];
  }

  // Destructor for StringArray
  virtual ~StringArray() {
    for (size_t i = 0; i < num_chunks_; i++) {
      delete[] elements_[i];
    }
    delete[] elements_;
  }

  // Resizes the array to the given size.
  virtual void _resize_to_(size_t new_capacity) {
    if ((new_capacity / CHUNK_SIZE) <= num_chunks_) {
      return;
    }

    size_t new_num_chunks = (new_capacity / CHUNK_SIZE) + 1;
    String ***new_elements = new String **[new_num_chunks];
    for (size_t i = 0; i < num_chunks_; i++) {
      if (i < new_num_chunks) {
        new_elements[i] = elements_[i];
      } else {
        delete[] elements_[i];
      }
    }

    for (size_t i = num_chunks_; i < new_num_chunks; i++) {
      new_elements[i] = new String *[CHUNK_SIZE];
    }

    delete[] elements_;
    elements_ = new_elements;
    capacity_ = CHUNK_SIZE * new_num_chunks;
    num_chunks_ = new_num_chunks;
    if (num_elements_ >= capacity_) {
      num_elements_ -= (num_elements_ - capacity_);
    }
  }

  // Determines if the array can fit the current number of elements.
  virtual void _grow_to_fit_(size_t num_elements) {
    if (num_elements > capacity_) {
      _resize_to_(num_elements * 2);
    }
  }

  // Appends e to end of array
  virtual void push_back(String *e) { add(num_elements_, e); }

  // Inserts e at index i
  virtual void add(size_t index, String *e) {
    _grow_to_fit_(num_elements_ + 1);
    for (size_t i = num_elements_; i > index; i--) {
      elements_[_row(i)][_col(i)] = elements_[_row(i - 1)][_col(i - 1)];
    }
    elements_[_row(index)][_col(index)] = e;
    num_elements_++;
  }

  // Adds elements in c to end of this array
  virtual void concat(StringArray *c) { add_all(num_elements_, c); }

  // Inserts all of elements in c into this array at index i
  virtual void add_all(size_t index, StringArray *c) {
    size_t len = c->size();
    size_t temp_num_elements = num_elements_;
    _grow_to_fit_(num_elements_ + len);
    for (size_t i = num_elements_ + len; i > index + len; i--) {
      size_t shift = i - len;
      elements_[_row(i)][_col(i)] = elements_[_row(shift)][_col(shift)];
    }

    // Add items from the given list.
    for (size_t i = index; i - index < len; i++) {
      elements_[_row(i)][_col(i)] = c->get(i - index);
      temp_num_elements++;
    }

    num_elements_ = temp_num_elements;
  }

  // Removes all of elements from this array
  virtual void clear() {
    _resize_to_(0);
    num_elements_ = 0;
  }

  // Compares o with this array for equality.
  virtual bool equals(Object *o) {
    StringArray *that = dynamic_cast<StringArray *>(o);
    if (that == nullptr) {
      return false;
    }
    if (that->size() != this->size()) {
      return false;
    }
    for (size_t i = 0; i < this->size(); i++) {
      String *ts = get(i);
      String *tt = that->get(i);
      if (ts == nullptr && tt == nullptr) {
        continue;
      } else if (ts == nullptr && tt != nullptr) {
        return false;
      } else if (ts != nullptr && tt == nullptr) {
        return false;
      } else if (!ts->equals(tt)) {
        return false;
      }
    }
    return true;
  }

  // Returns the element at index
  virtual String *get(size_t index) {
    return elements_[_row(index)][_col(index)];
  }

  // Helper functions for getting the corresponding row and col
  virtual size_t _row(size_t index) { return index / CHUNK_SIZE; }
  virtual size_t _col(size_t index) { return index % CHUNK_SIZE; }

  // Returns the hash code value for this array.
  virtual size_t hash() {
    size_t hash = 0;
    for (size_t i = 0; i < size(); i++) {
      String *o = get(i);
      if (o != nullptr) {
        hash += o->hash();
      }
    }
    return hash;
  }

  // Returns the index of the first occurrence of o, or number greater than
  // size() if not there
  virtual size_t index_of(Object *o) {
    for (size_t i = 0; i < size(); i++) {
      String *e = get(i);
      if (e == nullptr) {
        if (e == o) {
          return i;
        }
      } else if (e->equals(o)) {
        return i;
      }
    }
    return size() + 1;
  }

  // Removes the element at index
  virtual String *remove(size_t index) {
    String *old = get(index);
    for (size_t i = index; i < num_elements_; i++) {
      elements_[_row(i)][_col(i)] = elements_[_row(i + 1)][_col(i + 1)];
    }
    num_elements_--;
    return old;
  }

  // Replaces the element at index with e
  virtual String *set(size_t index, String *e) {
    String *old = get(index);
    elements_[_row(index)][_col(index)] = e;
    return old;
  }

  // Return the number of elements in the array
  virtual size_t size() { return num_elements_; }

  // Clones a deep copy of this String Array.
  virtual StringArray *clone() {
    StringArray *sa = new StringArray();
    for (size_t i = 0; i < size(); i++) {
      String *s = get(i);
      sa->push_back((s == nullptr) ? s : s->clone());
    }
    return sa;
  }
};

// /**
//  * @brief This is an Array class for primitive floats.
//  * It has the same time complexity as Array.
//  *
//  * @author griep.p@husky.neu.edu, colabella.a@husky.neu.edu
//  */
// class FloatArray : public Object {
// public:
//   size_t capacity_;
//   size_t num_chunks_;
//   size_t num_elements_;
//   float **elements_;

//   // Constructor for FloatArray
//   FloatArray() : Object() {
//     num_chunks_ = 1;
//     num_elements_ = 0;
//     capacity_ = CHUNK_SIZE;
//     elements_ = new float *[num_chunks_];
//     elements_[0] = new float[capacity_];
//   }

//   // Destructor for FloatArray
//   virtual ~FloatArray() {
//     for (size_t i = 0; i < num_chunks_; i++) {
//       delete[] elements_[i];
//     }
//     delete[] elements_;
//   }

//   // Resizes the array to the given size.
//   virtual void _resize_to_(size_t new_capacity) {
//     if ((new_capacity / CHUNK_SIZE) <= num_chunks_) {
//       return;
//     }

//     size_t new_num_chunks = (new_capacity / CHUNK_SIZE) + 1;
//     float **new_elements = new float *[new_num_chunks];
//     for (size_t i = 0; i < num_chunks_; i++) {
//       if (i < new_num_chunks) {
//         new_elements[i] = elements_[i];
//       } else {
//         delete[] elements_[i];
//       }
//     }

//     for (size_t i = num_chunks_; i < new_num_chunks; i++) {
//       new_elements[i] = new float[CHUNK_SIZE];
//     }

//     delete[] elements_;
//     elements_ = new_elements;
//     capacity_ = CHUNK_SIZE * new_num_chunks;
//     num_chunks_ = new_num_chunks;
//     if (num_elements_ >= capacity_) {
//       num_elements_ -= (num_elements_ - capacity_);
//     }
//   }

//   // Determines if the array can fit the current number of elements.
//   virtual void _grow_to_fit_(size_t num_elements) {
//     if (num_elements > capacity_) {
//       _resize_to_(num_elements * 2);
//     }
//   }

//   // Appends e to end of array
//   virtual void push_back(float e) { add(num_elements_, e); }

//   // Inserts e at index i
//   virtual void add(size_t index, float e) {
//     _grow_to_fit_(num_elements_ + 1);
//     for (size_t i = num_elements_; i > index; i--) {
//       elements_[_row(i)][_col(i)] = elements_[_row(i - 1)][_col(i - 1)];
//     }
//     elements_[_row(index)][_col(index)] = e;
//     num_elements_++;
//   }

//   // Adds elements in c to end of this array
//   virtual void concat(FloatArray *c) { add_all(num_elements_, c); }

//   // Inserts all of elements in c into this array at index i
//   virtual void add_all(size_t index, FloatArray *c) {
//     size_t len = c->size();
//     _grow_to_fit_(num_elements_ + len);

//     size_t temp_num_elements = num_elements_;
//     for (size_t i = num_elements_ + len; i > index + len; i--) {
//       size_t shift = i - len;
//       elements_[_row(i)][_col(i)] = elements_[_row(shift)][_col(shift)];
//     }

//     // Add items from the given list.
//     for (size_t i = index; i - index < len; i++) {
//       elements_[_row(i)][_col(i)] = c->get(i - index);
//       temp_num_elements++;
//     }

//     num_elements_ = temp_num_elements;
//   }

//   // Removes all of elements from this array
//   virtual void clear() {
//     _resize_to_(0);
//     num_elements_ = 0;
//   }

//   // Compares o with this array for equality.
//   virtual bool equals(Object *o) {
//     FloatArray *that = dynamic_cast<FloatArray *>(o);
//     if (that == nullptr)
//       return false;
//     if (that->size() != this->size())
//       return false;
//     for (size_t i = 0; i < this->size(); i++) {
//       if (this->get(i) != (that->get(i))) {
//         return false;
//       }
//     }
//     return true;
//   }

//   // Returns the element at index
//   virtual float get(size_t index) {
//     return elements_[_row(index)][_col(index)];
//   }

//   // Helper functions for getting the corresponding row and col
//   virtual size_t _row(size_t index) { return index / CHUNK_SIZE; }
//   virtual size_t _col(size_t index) { return index % CHUNK_SIZE; }

//   // Returns the hash code value for this array.
//   virtual size_t hash() {
//     size_t hash = 0;
//     for (size_t i = 0; i < size(); i++) {
//       hash += get(i);
//     }
//     return hash;
//   }

//   // Returns the index of the first occurrence of o, or number greater than
//   // size() if not there
//   virtual size_t index_of(float o) {
//     for (size_t i = 0; i < size(); i++) {
//       if (get(i) == o) {
//         return i;
//       }
//     }
//     return size() + 1;
//   }

//   // Removes the element at index
//   virtual float remove(size_t index) {
//     float old = get(index);
//     for (size_t i = index; i < num_elements_; i++) {
//       elements_[_row(i)][_col(i)] = elements_[_row(i + 1)][_col(i + 1)];
//     }
//     num_elements_--;
//     return old;
//   }

//   // Replaces the element at index i with e
//   virtual float set(size_t index, float e) {
//     float old = get(index);
//     elements_[_row(index)][_col(index)] = e;
//     return old;
//   }

//   // Return the number of elements in the array
//   virtual size_t size() { return num_elements_; }
// };

// /**
//  * @brief This is an Array class for primitive integers.
//  * It has the same time complexity as Array.
//  *
//  * @author griep.p@husky.neu.edu, colabella.a@husky.neu.edu
//  */
// class IntArray : public Object {
// public:
//   size_t capacity_;
//   size_t num_chunks_;
//   size_t num_elements_;
//   int **elements_;

//   // Constructor for IntArray
//   IntArray() {
//     num_chunks_ = 1;
//     num_elements_ = 0;
//     capacity_ = CHUNK_SIZE;
//     elements_ = new int *[num_chunks_];
//     elements_[0] = new int[CHUNK_SIZE];
//   }

//   // Destructor for IntArray
//   virtual ~IntArray() {
//     for (size_t i = 0; i < num_chunks_; i++) {
//       delete[](elements_[i]);
//     }
//     delete[] elements_;
//   }

//   // Resizes the array to the given size.
//   virtual void _resize_to_(size_t new_capacity) {
//     if ((new_capacity / CHUNK_SIZE) <= num_chunks_) {
//       return;
//     }

//     size_t new_num_chunks = (new_capacity / CHUNK_SIZE) + 1;
//     int **new_elements = new int *[new_num_chunks];
//     for (size_t i = 0; i < num_chunks_; i++) {
//       if (i < new_num_chunks) {
//         new_elements[i] = elements_[i];
//       } else {
//         delete[] elements_[i];
//       }
//     }

//     for (size_t i = num_chunks_; i < new_num_chunks; i++) {
//       new_elements[i] = new int[CHUNK_SIZE];
//     }

//     delete[] elements_;
//     elements_ = new_elements;
//     capacity_ = CHUNK_SIZE * new_num_chunks;
//     num_chunks_ = new_num_chunks;
//     if (num_elements_ >= capacity_) {
//       num_elements_ -= (num_elements_ - capacity_);
//     }
//   }

//   // Determines if the array can fit the current number of elements.
//   virtual void _grow_to_fit_(size_t num_elements) {
//     if (num_elements > capacity_) {
//       _resize_to_(num_elements * 2);
//     }
//   }

//   // Appends e to end of array
//   virtual void push_back(int e) { add(num_elements_, e); }

//   // Inserts e at index i
//   virtual void add(size_t index, int e) {
//     _grow_to_fit_(num_elements_ + 1);
//     for (size_t i = num_elements_; i > index; i--) {
//       elements_[_row(i)][_col(i)] = elements_[_row(i - 1)][_col(i - 1)];
//     }
//     elements_[_row(index)][_col(index)] = e;
//     num_elements_++;
//   }

//   // Adds elements in c to end of this array
//   virtual void concat(IntArray *c) { add_all(num_elements_, c); }

//   // Inserts all of elements in c into this array at index i
//   virtual void add_all(size_t index, IntArray *c) {
//     size_t len = c->size();
//     _grow_to_fit_(num_elements_ + len);

//     size_t temp_num_elements = num_elements_;
//     for (size_t i = num_elements_ + len; i > index + len; i--) {
//       size_t shift = i - len;
//       elements_[_row(i)][_col(i)] = elements_[_row(shift)][_col(shift)];
//     }

//     // Add items from the given list.
//     for (size_t i = index; i - index < len; i++) {
//       elements_[_row(i)][_col(i)] = c->get(i - index);
//       temp_num_elements++;
//     }

//     num_elements_ = temp_num_elements;
//   }

//   // Removes all of elements from this array
//   virtual void clear() {
//     _resize_to_(0);
//     num_elements_ = 0;
//   }

//   // Compares o with this array for equality.
//   virtual bool equals(Object *o) {
//     IntArray *that = dynamic_cast<IntArray *>(o);
//     if (that == nullptr)
//       return false;
//     if (that->size() != this->size())
//       return false;
//     for (size_t i = 0; i < this->size(); i++) {
//       if (this->get(i) != (that->get(i))) {
//         return false;
//       }
//     }
//     return true;
//   }

//   // Returns the element at index
//   virtual int get(size_t index) { return elements_[_row(index)][_col(index)];
//   }

//   // Helper functions for getting the corresponding row and col
//   virtual size_t _row(size_t index) { return index / CHUNK_SIZE; }
//   virtual size_t _col(size_t index) { return index % CHUNK_SIZE; }

//   // Returns the hash code value for this array.
//   virtual size_t hash() {
//     size_t hash = 0;
//     for (size_t i = 0; i < size(); i++) {
//       hash += get(i);
//     }
//     return hash;
//   }

//   // Returns the index of the first occurrence of o, or number greater than
//   // size() if not there
//   virtual size_t index_of(int o) {
//     for (size_t i = 0; i < size(); i++) {
//       if (get(i) == o) {
//         return i;
//       }
//     }
//     return size() + 1;
//   }

//   // Removes the element at index
//   virtual int remove(size_t index) {
//     int old = get(index);
//     for (size_t i = index; i < num_elements_; i++) {
//       elements_[_row(i)][_col(i)] = elements_[_row(i + 1)][_col(i + 1)];
//     }
//     num_elements_--;
//     return old;
//   }

//   // Replaces the element at index i with e
//   virtual int set(size_t index, int e) {
//     int old = get(index);
//     elements_[_row(index)][_col(index)] = e;
//     return old;
//   }

//   // Return the number of elements in the array
//   virtual size_t size() { return num_elements_; }
// };

// /**
//  * @brief This is an Array class for primitive booleans.
//  * It has the same time complexity as Array.
//  *
//  * @author griep.p@husky.neu.edu, colabella.a@husky.neu.edu
//  */
// class BoolArray : public Object {
// public:
//   size_t capacity_;
//   size_t num_chunks_;
//   size_t num_elements_;
//   bool **elements_;

//   // Constructor for BoolArray
//   BoolArray() : Object() {
//     num_chunks_ = 1;
//     num_elements_ = 0;
//     capacity_ = CHUNK_SIZE;
//     elements_ = new bool *[num_chunks_];
//     elements_[0] = new bool[capacity_];
//   }

//   // Destructor for BoolArray
//   virtual ~BoolArray() {
//     for (size_t i = 0; i < num_chunks_; i++) {
//       delete[] elements_[i];
//     }
//     delete[] elements_;
//   }

//   // Resizes the array to the given size.
//   virtual void _resize_to_(size_t new_capacity) {
//     if ((new_capacity / CHUNK_SIZE) <= num_chunks_) {
//       return;
//     }

//     size_t new_num_chunks = (new_capacity / CHUNK_SIZE) + 1;
//     bool **new_elements = new bool *[new_num_chunks];
//     for (size_t i = 0; i < num_chunks_; i++) {
//       if (i < new_num_chunks) {
//         new_elements[i] = elements_[i];
//       } else {
//         delete[] elements_[i];
//       }
//     }

//     for (size_t i = num_chunks_; i < new_num_chunks; i++) {
//       new_elements[i] = new bool[CHUNK_SIZE];
//     }

//     delete[] elements_;
//     elements_ = new_elements;
//     capacity_ = CHUNK_SIZE * new_num_chunks;
//     num_chunks_ = new_num_chunks;
//     if (num_elements_ >= capacity_) {
//       num_elements_ -= (num_elements_ - capacity_);
//     }
//   }

//   // Determines if the array can fit the current number of elements.
//   virtual void _grow_to_fit_(size_t num_elements) {
//     if (num_elements > capacity_) {
//       _resize_to_(num_elements * 2);
//     }
//   }

//   // Appends e to end of array
//   virtual void push_back(bool e) { add(num_elements_, e); }

//   // Inserts e at index i
//   virtual void add(size_t index, bool e) {
//     _grow_to_fit_(num_elements_ + 1);
//     for (size_t i = num_elements_; i > index; i--) {
//       elements_[_row(i)][_col(i)] = elements_[_row(i - 1)][_col(i - 1)];
//     }
//     elements_[_row(index)][_col(index)] = e;
//     num_elements_++;
//   }

//   // Adds elements in c to end of this array
//   virtual void concat(BoolArray *c) { add_all(num_elements_, c); }

//   // Inserts all of elements in c into this array at index i
//   virtual void add_all(size_t index, BoolArray *c) {
//     size_t len = c->size();
//     _grow_to_fit_(num_elements_ + len);
//     size_t temp_num_elements = num_elements_;

//     for (size_t i = num_elements_ + len; i > index + len; i--) {
//       size_t shift = i - len;
//       elements_[_row(i)][_col(i)] = elements_[_row(shift)][_col(shift)];
//     }

//     // Add items from the given list.
//     for (size_t i = index; i - index < len; i++) {
//       elements_[_row(i)][_col(i)] = c->get(i - index);
//       temp_num_elements++;
//     }

//     num_elements_ = temp_num_elements;
//   }

//   // Removes all of elements from this array
//   virtual void clear() {
//     _resize_to_(0);
//     num_elements_ = 0;
//   }

//   // Compares o with this array for equality.
//   virtual bool equals(Object *o) {
//     BoolArray *that = dynamic_cast<BoolArray *>(o);
//     if (that == nullptr)
//       return false;
//     if (that->size() != this->size())
//       return false;
//     for (size_t i = 0; i < this->size(); i++) {
//       if (this->get(i) != (that->get(i))) {
//         return false;
//       }
//     }
//     return true;
//   }

//   // Returns the element at index
//   virtual bool get(size_t index) { return
//   elements_[_row(index)][_col(index)]; }

//   // Helper functions for getting the corresponding row and col
//   virtual size_t _row(size_t index) { return index / CHUNK_SIZE; }
//   virtual size_t _col(size_t index) { return index % CHUNK_SIZE; }

//   // Returns the hash code value for this array.
//   virtual size_t hash() {
//     size_t hash = 0;
//     for (size_t i = 0; i < size(); i++) {
//       hash += get(i);
//     }
//     return hash;
//   }

//   // Returns the index of the first occurrence of o, or number greater than
//   // size() if not there
//   virtual size_t index_of(int o) {
//     for (size_t i = 0; i < size(); i++) {
//       if (get(i) == o) {
//         return i;
//       }
//     }
//     return size() + 1;
//   }

//   // Removes the element at index
//   virtual bool remove(size_t index) {
//     bool old = get(index);
//     for (size_t i = index; i < num_elements_; i++) {
//       elements_[_row(i)][_col(i)] = elements_[_row(i + 1)][_col(i + 1)];
//     }
//     num_elements_--;
//     return old;
//   }

//   // Replaces the element at index i with e
//   virtual bool set(size_t index, int e) {
//     bool old = get(index);
//     elements_[_row(index)][_col(index)] = e;
//     return old;
//   }

//   // Return the number of elements in the array
//   virtual size_t size() { return num_elements_; }
// };

// /**
//  * @brief This is an Array class for primitive doubles.
//  * It has the same time complexity as Array.
//  *
//  * @author griep.p@husky.neu.edu, colabella.a@husky.neu.edu
//  */
// class DoubleArray : public Object {
// public:
//   size_t capacity_;
//   size_t num_chunks_;
//   size_t num_elements_;
//   double **elements_;

//   // Constructor for DoubleArray
//   DoubleArray() : Object() {
//     num_chunks_ = 1;
//     num_elements_ = 0;
//     capacity_ = CHUNK_SIZE;
//     elements_ = new double *[num_chunks_];
//     elements_[0] = new double[capacity_];
//   }

//   // Destructor for DoubleArray
//   virtual ~DoubleArray() {
//     for (size_t i = 0; i < num_chunks_; i++) {
//       delete[] elements_[i];
//     }
//     delete[] elements_;
//   }

//   // Resizes the array to the given size.
//   virtual void _resize_to_(size_t new_capacity) {
//     if ((new_capacity / CHUNK_SIZE) <= num_chunks_) {
//       return;
//     }

//     size_t new_num_chunks = (new_capacity / CHUNK_SIZE) + 1;
//     double **new_elements = new double *[new_num_chunks];
//     for (size_t i = 0; i < num_chunks_; i++) {
//       if (i < new_num_chunks) {
//         new_elements[i] = elements_[i];
//       } else {
//         delete[] elements_[i];
//       }
//     }

//     for (size_t i = num_chunks_; i < new_num_chunks; i++) {
//       new_elements[i] = new double[CHUNK_SIZE];
//     }

//     delete[] elements_;
//     elements_ = new_elements;
//     capacity_ = CHUNK_SIZE * new_num_chunks;
//     num_chunks_ = new_num_chunks;
//     if (num_elements_ >= capacity_) {
//       num_elements_ -= (num_elements_ - capacity_);
//     }
//   }

//   // Determines if the array can fit the current number of elements.
//   virtual void _grow_to_fit_(size_t num_elements) {
//     if (num_elements > capacity_) {
//       _resize_to_(num_elements * 2);
//     }
//   }

//   // Appends e to end of array
//   virtual void push_back(float e) { add(num_elements_, e); }

//   // Inserts e at index i
//   virtual void add(size_t index, double e) {
//     _grow_to_fit_(num_elements_ + 1);
//     for (size_t i = num_elements_; i > index; i--) {
//       elements_[_row(i)][_col(i)] = elements_[_row(i - 1)][_col(i - 1)];
//     }
//     elements_[_row(index)][_col(index)] = e;
//     num_elements_++;
//   }

//   // Adds elements in c to end of this array
//   virtual void concat(DoubleArray *c) { add_all(num_elements_, c); }

//   // Inserts all of elements in c into this array at index i
//   virtual void add_all(size_t index, DoubleArray *c) {
//     size_t len = c->size();
//     _grow_to_fit_(num_elements_ + len);

//     size_t temp_num_elements = num_elements_;
//     for (size_t i = num_elements_ + len; i > index + len; i--) {
//       size_t shift = i - len;
//       elements_[_row(i)][_col(i)] = elements_[_row(shift)][_col(shift)];
//     }

//     // Add items from the given list.
//     for (size_t i = index; i - index < len; i++) {
//       elements_[_row(i)][_col(i)] = c->get(i - index);
//       temp_num_elements++;
//     }

//     num_elements_ = temp_num_elements;
//   }

//   // Removes all of elements from this array
//   virtual void clear() {
//     _resize_to_(0);
//     num_elements_ = 0;
//   }

//   // Compares o with this array for equality.
//   virtual bool equals(Object *o) {
//     DoubleArray *that = dynamic_cast<DoubleArray *>(o);
//     if (that == nullptr)
//       return false;
//     if (that->size() != this->size())
//       return false;
//     for (size_t i = 0; i < this->size(); i++) {
//       if (this->get(i) - (that->get(i)) > DOUBLE_TOLERANCE) {
//         return false;
//       } else if (that->get(i) - this->get(i) > DOUBLE_TOLERANCE) {
//         return false;
//       }
//     }
//     return true;
//   }

//   // Returns the element at index
//   virtual double get(size_t index) {
//     return elements_[_row(index)][_col(index)];
//   }

//   // Helper functions for getting the corresponding row and col
//   virtual size_t _row(size_t index) { return index / CHUNK_SIZE; }
//   virtual size_t _col(size_t index) { return index % CHUNK_SIZE; }

//   // Returns the hash code value for this array.
//   virtual size_t hash() {
//     size_t hash = 0;
//     for (size_t i = 0; i < size(); i++) {
//       hash += get(i);
//     }
//     return hash;
//   }

//   // Returns the index of the first occurrence of o, or number greater than
//   // size() if not there
//   virtual size_t index_of(double o) {
//     for (size_t i = 0; i < size(); i++) {
//       if (get(i) == o) {
//         return i;
//       }
//     }
//     return size() + 1;
//   }

//   // Removes the element at index
//   virtual double remove(size_t index) {
//     double old = get(index);
//     for (size_t i = index; i < num_elements_; i++) {
//       elements_[_row(i)][_col(i)] = elements_[_row(i + 1)][_col(i + 1)];
//     }
//     num_elements_--;
//     return old;
//   }

//   // Replaces the element at index i with e
//   virtual double set(size_t index, double e) {
//     double old = get(index);
//     elements_[_row(index)][_col(index)] = e;
//     return old;
//   }

//   // Return the number of elements in the array
//   virtual size_t size() { return num_elements_; }
// };
