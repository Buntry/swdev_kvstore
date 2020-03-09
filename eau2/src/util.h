#pragma once
// lang:CwC

#include "object.h"

static const double DOUBLE_TOLERANCE = 0.0001;

class Util : public Object {
public:
  /** Equality functions **/
  static bool equals(Object *a, Object *b) {
    if (a == nullptr && b == nullptr) {
      return true;
    } else if (a == nullptr && b != nullptr) {
      return false;
    } else if (a != nullptr && b == nullptr) {
      return false;
    } else {
      return a->equals(b);
    }
  }

  static bool equals(int a, int b) { return a == b; }
  static bool equals(float a, float b) { return a == b; }
  static bool equals(double a, double b) {
    if (a - b > DOUBLE_TOLERANCE) {
      return false;
    } else if (b - a > DOUBLE_TOLERANCE) {
      return false;
    } else {
      return true;
    }
  }
  static bool equals(bool a, bool b) { return a == b; }

  /** Hash functions **/
  static size_t hash(Object *a) { return (a == nullptr) ? 0 : a->hash(); }
  static size_t hash(int a) { return a; }
  static size_t hash(float a) { return std::hash<float>{}(a); }   // lang: Cpp
  static size_t hash(double a) { return std::hash<double>{}(a); } // lang: Cpp
  static size_t hash(bool a) { return a; }
};

/**
 * #define gen_array(KlassArray, Stores) \
  class KlassArray : public Object {                                           \
  public:                                                                      \
    size_t num_chunks_ = 1;                                                    \
    size_t num_elements_ = 0;                                                  \
    size_t capacity_ = CHUNK_SIZE;                                             \
    Stores **elements_ = new Stores *[num_chunks_];                            \
    elements_[0] = new Stores[capacity_];                                      \
    \
    ~KlassArray() {                                                            \
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
      if (num_elements >= capacity_) {                                         \
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
**/
