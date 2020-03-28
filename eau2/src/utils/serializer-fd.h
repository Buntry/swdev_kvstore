#pragma once
// lang: CwC

// Forward declaration of Object
class Object;

// Forward declaration of CharArray
class CharArray;

/** Represents a Serializer that can take a Message and
 * serialize its contents.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Serializer {
public:
  CharArray *data_;

  Serializer();
  ~Serializer();

  /** Write methods **/
  void write(Object *o);
  void write(size_t x);
  void write(char x);
  void write(char *x, size_t len);
  void write(bool x);
  void write(int x);
  void write(float x);
  void write(double x);

  /** Informational methods **/
  size_t num_chunks();
  char *get_chunk(size_t index);
  CharArray *data();
  size_t length();
};

/** Represents a Deserializer that can read from serialized data.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Deserializer {
public:
  CharArray *data_;
  size_t cursor_ = 0;

  Deserializer(CharArray *data);
  Deserializer(CharArray &data);
  ~Deserializer();

  /** Peek at the first value **/
  size_t peek_size_t();

  size_t read_size_t();
  char read_char();
  char *read_chars(size_t len);
  bool read_bool();
  int read_int();
  float read_float();
  double read_double();

  size_t incr_cursor_();
  char next_char_();
  void fill_up_(char *bytes, size_t len);
  CharArray *data() { return data_; }
};
