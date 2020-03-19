// lang: CwC
#pragma once

#include "../client/message.h"
#include "../store/dataframe.h"

/** Represents the types of messages a node can send over the network. **/
enum class ValKind {
  String,
  BoolArray,
  CharArray,
  IntArray,
  FloatArray,
  StringArray,
  BoolColumn,
  IntColumn,
  FloatColumn,
  StringColumn,
  Schema,
  DataFrame
};

/** Represents a Serializer that can take a Message and
 * serialize its contents.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Serializer : public Object {
public:
  CharArray data_;

  /** Writes an Object to this serializer **/
  void write_(Object *o) { o->serialize(*this); }

  /** Writes a size_t to this serializer **/
  void write(size_t v) {
    char *cv = reinterpret_cast<char *>(&v);
    for (size_t i = 0; i < sizeof(size_t); i++) {
      data_.push_back(cv[i]);
    }
  }

  /** Writes an integer to this serializer **/
  void write(int v) {
    char *cv = reinterpret_cast<char *>(&v);
    for (size_t i = 0; i < sizeof(int); i++) {
      data_.push_back(cv[i]);
    }
  }

  /** Writes a char to this serializer **/
  void write(char c) {
    char *cv = reinterpret_cast<char *>(&c);
    for (size_t i = 0; i < sizeof(char); i++) {
      data_.push_back(cv[i]);
    }
  }

  /** Writes a boolean to this serializer **/
  void write(bool b) {
    char *cv = reinterpret_cast<char *>(&b);
    for (size_t i = 0; i < sizeof(bool); i++) {
      data_.push_back(cv[i]);
    }
  }

  /** Writes a float to this serializer **/
  void write(float f) {
    char *cv = reinterpret_cast<char *>(&f);
    for (size_t i = 0; i < sizeof(float); i++) {
      data_.push_back(cv[i]);
    }
  }

  /** Writes a double to this serializer **/
  void write(double d) {
    char *cv = reinterpret_cast<char *>(&d);
    for (size_t i = 0; i < sizeof(double); i++) {
      data_.push_back(cv[i]);
    }
  }

  /** Writes a character array to this serializer **/
  void write(char *arr, size_t len) {
    for (size_t i = 0; i < len; i++) {
      data_.push_back(arr[i]);
    }
  }

  /** Gets the data (read-only) **/
  CharArray *data() { return &data_; }
};

/** Represents a Deserializer that can read from serialized data.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Deserializer : public Object {
public:
  CharArray *data_;
  size_t cursor_ = 0;

  Deserializer(CharArray *data) : data_(data) {}
  ~Deserializer() { delete data_; }

  size_t incr_cursor() {
    assert(cursor_ < data_->size());
    return cursor_++;
  }

  char next() { return data_->get(incr_cursor()); }

  size_t read_size_t() {
    char v[sizeof(size_t)];
    for (size_t i = 0; i < sizeof(size_t); i++) {
      v[i] = next();
    }
    return reinterpret_cast<size_t>(v);
  }

  char *read_chars(size_t length) {}

  size_t peek_size_t() {
    assert(sizeof(size_t) + cursor_ <= data_->size());

    char v[sizeof(size_t)];
    for (size_t i = 0; i < sizeof(size_t) && cursor_ + i < data_->size(); i++) {
      v[i] = data_->get(cursor_ + i);
    }
    return reinterpret_cast<size_t>(v);
  }

  Object *read_object() { ValKind vkind = static_cast<ValKind>(peek_size_t()); }
};
