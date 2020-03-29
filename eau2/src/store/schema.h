// lang: CwC
#pragma once

#include "column.h"

/*************************************************************************
 * Schema::
 * A schema is a description of the contents of a data frame, the schema
 * knows the number of columns and number of rows, the type of each column,
 * optionally columns and rows can be named by strings.
 * The valid types are represented by the chars 'S', 'B', 'I' and 'F'.
 *
 * In order to encapsulate distributed schemas, DataFrames also need to know
 * their current chunk index.
 *
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu
 */
class Schema : public Object {
public:
  CharArray *types_;
  SizeTArray *chunk_indexes_;
  size_t width_ = 0;
  size_t height_ = 0;

  /** Copying constructor */
  Schema(Schema &from) {
    types_ = dynamic_cast<CharArray *>(Util::clone(from.types_));
    chunk_indexes_ =
        dynamic_cast<SizeTArray *>(Util::clone(from.chunk_indexes_));
    width_ = from.width_;
  }

  /** Create an empty schema **/
  Schema() {
    types_ = new CharArray();
    chunk_indexes_ = new SizeTArray();
  }

  /** Create a schema from a string of types. A string that contains
   * characters other than those identifying the four type results in
   * undefined behavior. The argument is external, a nullptr argument is
   * undefined. **/
  Schema(const char *types) {
    types_ = new CharArray();
    chunk_indexes_ = new SizeTArray();

    for (size_t i = 0; i < strlen(types); i++) {
      add_column(types[i]);
    }
  }

  /** Destructor for Schema.
   * This implementation assume that even though the arguments to a Schema
   * are external, the Schema acquires ownership of everything in its
   * arrays. **/
  ~Schema() {
    delete types_;
    delete chunk_indexes_;
  }

  /** Add a column of the given type and name (can be nullptr), name
   * is external. Names are expectd to be unique, duplicates result
   * in undefined behavior. */
  void add_column(char typ) {
    types_->push_back(typ);
    chunk_indexes_->push_back(0);
    width_++;
  }

  /** Add a row with a name (possibly nullptr), name is external.  Names
   * are expectd to be unique, duplicates result in undefined behavior. */
  void add_row() { height_++; }

  /** Return type of column at col. An col >= width is undefined. */
  char col_type(size_t col) { return types_->get(col); }

  /** Return the current chunk index for a given column. **/
  size_t chunk_index(size_t col) { return chunk_indexes_->get(col); }
  void loaded_index(size_t col, size_t idx) { chunk_indexes_->set(col, idx); }

  /** The number of columns */
  size_t width() { return width_; }

  /** The number of rows */
  size_t length() { return height_; }

  /** Schema equality is purely based on column type. **/
  bool equals(Object *other) {
    Schema *that = dynamic_cast<Schema *>(other);
    return (that != nullptr) && (this->width() == that->width()) &&
           types_->equals(that->types_);
  }

  /** Hash function is redefined st. if two schemas are equal,
   * then they must have the same hash. **/
  size_t hash() {
    size_t hash = 0;
    for (size_t i = 0; i < this->width(); i++) {
      hash += col_type(i);
    }
    return hash;
  }

  /** Serializes a schema onto an object. Does not serialize row/col names. **/
  void serialize(Serializer &ser) {
    ser.write(width_);
    ser.write(height_);
    for (size_t i = 0; i < width(); i++) {
      ser.write(col_type(i));
    }
  }

  /** Deserializes a schema from a deserializer. **/
  static Schema *deserialize(Deserializer &dser) {
    Schema *schema = new Schema();
    schema->width_ = dser.read_size_t();
    schema->height_ = dser.read_size_t();
    for (size_t i = 0; i < schema->width_; i++) {
      schema->types_->push_back(dser.read_char());
    }
    return schema;
  }
};