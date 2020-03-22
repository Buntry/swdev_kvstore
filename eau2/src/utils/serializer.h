// lang: CwC
#pragma once

#include "array.h"
#include "serializer-fd.h"

/** Serializer constructor/destructor **/
Serializer::Serializer() { data_ = new CharArray(); }
Serializer::~Serializer() { delete data_; }

/** Writes an Object to this serializer **/
void Serializer::write(Object *o) { o->serialize(*this); }

/** Writes a size_t to this serializer **/
void Serializer::write(size_t v) {
  char *cv = reinterpret_cast<char *>(&v);
  for (size_t i = 0; i < sizeof(size_t); i++) {
    data_->push_back(cv[i]);
  }
}

/** Writes an integer to this serializer **/
void Serializer::write(int v) {
  char *cv = reinterpret_cast<char *>(&v);
  for (size_t i = 0; i < sizeof(int); i++) {
    data_->push_back(cv[i]);
  }
}

/** Writes a char to this serializer **/
void Serializer::write(char c) { data_->push_back(c); }

/** Writes a boolean to this serializer **/
void Serializer::write(bool b) {
  char *cv = reinterpret_cast<char *>(&b);
  for (size_t i = 0; i < sizeof(bool); i++) {
    data_->push_back(cv[i]);
  }
}

/** Writes a float to this serializer **/
void Serializer::write(float f) {
  char *cv = reinterpret_cast<char *>(&f);
  for (size_t i = 0; i < sizeof(float); i++) {
    data_->push_back(cv[i]);
  }
}

/** Writes a double to this serializer **/
void Serializer::write(double d) {
  char *cv = reinterpret_cast<char *>(&d);
  for (size_t i = 0; i < sizeof(double); i++) {
    data_->push_back(cv[i]);
  }
}

/** Writes a character array to this serializer **/
void Serializer::write(char *arr, size_t len) {
  for (size_t i = 0; i < len; i++)
    data_->push_back(arr[i]);
}

/** Gets the data (read-only) **/
CharArray *Serializer::data() { return data_; }
size_t Serializer::length() { return data_->size(); }

/** Gets the internal details and chunks of the serializer.
 * (Warning) this uses field-of-field and is poorly designed. **/
size_t Serializer::num_chunks() { return data_->num_chunks_; }
char *Serializer::get_chunk(size_t index) { return data_->elements_[index]; }

/** Deserializer implementation **/

/** Constructors and deconstructors **/
Deserializer::Deserializer(CharArray *data) { data_ = data; }
Deserializer::Deserializer(CharArray &data) { data_ = data.clone(); }
Deserializer::~Deserializer() { delete data_; }

/** Peek at the first value **/
size_t Deserializer::peek_size_t() {
  assert(sizeof(size_t) + cursor_ <= data_->size());
  size_t v;
  char *cv = reinterpret_cast<char *>(&v);
  for (size_t i = 0; i < sizeof(size_t) && cursor_ + i < data_->size(); i++) {
    cv[i] = data_->get(cursor_ + i);
  }
  return v;
}

/** Read a size_t **/
size_t Deserializer::read_size_t() {
  size_t v;
  fill_up_(reinterpret_cast<char *>(&v), sizeof(size_t));
  return v;
}

/** Read a single character **/
char Deserializer::read_char() { return next_char_(); }

/** Read a character array **/
char *Deserializer::read_chars(size_t len) {
  char *arr = new char[len];
  fill_up_(arr, len);
  return arr;
}

/** Read a boolean **/
bool Deserializer::read_bool() {
  bool b;
  fill_up_(reinterpret_cast<char *>(&b), sizeof(bool));
  return b;
}

/** Read an integer **/
int Deserializer::read_int() {
  int v;
  fill_up_(reinterpret_cast<char *>(&v), sizeof(int));
  return v;
}

/** Read a float **/
float Deserializer::read_float() {
  float v;
  fill_up_(reinterpret_cast<char *>(&v), sizeof(float));
  return v;
}

/** Read a double **/
double Deserializer::read_double() {
  double v;
  fill_up_(reinterpret_cast<char *>(&v), sizeof(double));
  return v;
}

/** Safely increments the cursor **/
size_t Deserializer::incr_cursor_() {
  assert(cursor_ < data_->size());
  return cursor_++;
}

/** Gets the next character **/
char Deserializer::next_char_() { return data_->get(incr_cursor_()); }

/** Fills up the given buffer. **/
void Deserializer::fill_up_(char *bytes, size_t len) {
  for (size_t i = 0; i < len; i++) {
    bytes[i] = next_char_();
  }
}