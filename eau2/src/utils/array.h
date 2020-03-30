// lang: CwC
#pragma once
#include <assert.h>

#include "object.h"
#include "string.h"
#include "util.h"

static const size_t CHUNK_SIZE = 16384;

#define generate_classarray(KlassArray, Stores)                                \
  class KlassArray : public Object {                                           \
  public:                                                                      \
    size_t num_chunks_;                                                        \
    size_t num_elements_;                                                      \
    size_t capacity_;                                                          \
    size_t start_pos_;                                                         \
    Stores **elements_;                                                        \
                                                                               \
    KlassArray() : Object() {                                                  \
      num_chunks_ = 1;                                                         \
      num_elements_ = 0;                                                       \
      start_pos_ = 0;                                                          \
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
                                                                               \
    void grow_to_fit_(size_t num_elements) {                                   \
      if (num_elements <= capacity_)                                           \
        return;                                                                \
      size_t new_num_chunks = ((num_elements * 2) / CHUNK_SIZE) + 1;           \
      if (new_num_chunks <= num_chunks_)                                       \
        return;                                                                \
      Stores **new_elements = new Stores *[new_num_chunks];                    \
      for (size_t i = 0; i < new_num_chunks; i++) {                            \
        if (i < num_chunks_) {                                                 \
          new_elements[i] = elements_[chunk_offset_(i)];                       \
        } else {                                                               \
          new_elements[i] = new Stores[CHUNK_SIZE];                            \
        }                                                                      \
      }                                                                        \
      for (size_t i = 0; i < CHUNK_SIZE; i++) {                                \
        new_elements[num_chunks_][i] = elements_[chunk_offset_(0)][i];         \
      }                                                                        \
      delete[] elements_;                                                      \
      elements_ = new_elements;                                                \
      capacity_ = CHUNK_SIZE * new_num_chunks;                                 \
      num_chunks_ = new_num_chunks;                                            \
      start_pos_ %= CHUNK_SIZE;                                                \
    }                                                                          \
                                                                               \
    virtual void push_back(Stores e) { add(num_elements_, e); }                \
                                                                               \
    virtual void add(size_t index, Stores e) {                                 \
      grow_to_fit_(num_elements_ + 1);                                         \
      for (size_t i = num_elements_; i > index; i--) {                         \
        elements_[row_(i)][col_(i)] = elements_[row_(i - 1)][col_(i - 1)];     \
      }                                                                        \
      elements_[row_(index)][col_(index)] = e;                                 \
      num_elements_++;                                                         \
    }                                                                          \
                                                                               \
    virtual void concat(KlassArray *c) { add_all(num_elements_, c); }          \
                                                                               \
    virtual void add_all(size_t index, KlassArray *c) {                        \
      size_t len = c->size();                                                  \
      size_t temp_num_elements = num_elements_;                                \
      grow_to_fit_(num_elements_ + len);                                       \
      for (size_t i = num_elements_ + len; i > index + len; i--) {             \
        size_t shift = i - len;                                                \
        elements_[row_(i)][col_(i)] = elements_[row_(shift)][col_(shift)];     \
      }                                                                        \
      for (size_t i = index; i - index < len; i++) {                           \
        elements_[row_(i)][col_(i)] = c->get(i - index);                       \
        temp_num_elements++;                                                   \
      }                                                                        \
      num_elements_ = temp_num_elements;                                       \
    }                                                                          \
                                                                               \
    virtual void clear() { num_elements_ = 0; }                                \
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
    virtual Stores get(size_t index) {                                         \
      return elements_[row_(index)][col_(index)];                              \
    }                                                                          \
    virtual size_t offset_(size_t index) {                                     \
      return (index + start_pos_) % capacity_;                                 \
    }                                                                          \
    virtual size_t chunk_offset_(size_t chunk_index) {                         \
      return (row_(0) + chunk_index) % num_chunks_;                            \
    }                                                                          \
    virtual size_t row_(size_t index) { return offset_(index) / CHUNK_SIZE; }  \
    virtual size_t col_(size_t index) { return offset_(index) % CHUNK_SIZE; }  \
                                                                               \
    virtual size_t hash() {                                                    \
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
        elements_[row_(i)][col_(i)] = elements_[row_(i + 1)][col_(i + 1)];     \
      }                                                                        \
      num_elements_--;                                                         \
      return old;                                                              \
    }                                                                          \
                                                                               \
    virtual Stores set(size_t index, Stores e) {                               \
      Stores old = get(index);                                                 \
      elements_[row_(index)][col_(index)] = e;                                 \
      return old;                                                              \
    }                                                                          \
                                                                               \
    virtual size_t size() { return num_elements_; }                            \
                                                                               \
    virtual KlassArray *clone() {                                              \
      KlassArray *ka = new KlassArray();                                       \
      for (size_t i = 0; i < size(); i++) {                                    \
        ka->push_back(Util::clone(get(i)));                                    \
      }                                                                        \
      return ka;                                                               \
    }                                                                          \
  }

generate_classarray(Array, Object *);
generate_classarray(BoolArray, bool);
generate_classarray(IntArray, int);
generate_classarray(FloatArray, float);
generate_classarray(DoubleArray, double);
generate_classarray(CharArray, char);
generate_classarray(SizeTArray, size_t);

#define generate_object_classarray(KlassArray, Klass)                          \
  class KlassArray : public Array {                                            \
  public:                                                                      \
    virtual Klass *get(size_t index) {                                         \
      return dynamic_cast<Klass *>(Array::get(index));                         \
    }                                                                          \
    using Array::set;                                                          \
    virtual Klass *set(size_t index, Klass *v) {                               \
      return dynamic_cast<Klass *>(Array::set(index, v));                      \
    }                                                                          \
    virtual Klass *remove(size_t index) {                                      \
      return dynamic_cast<Klass *>(Array::remove(index));                      \
    }                                                                          \
    virtual KlassArray *clone() {                                              \
      KlassArray *ka = new KlassArray();                                       \
      for (size_t i = 0; i < size(); i++) {                                    \
        ka->push_back(Util::clone(get(i)));                                    \
      }                                                                        \
      return ka;                                                               \
    }                                                                          \
  }

generate_object_classarray(StringArray, String);