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
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu
 */
class Schema : public Object {
public:
  CharArray *types_;
  StringArray *rows_;
  StringArray *cols_;

  /** Copying constructor */
  Schema(Schema &from) {
    types_ = from.types_->clone();
    rows_ = from.rows_->clone();
    cols_ = from.cols_->clone();
  }

  /** Create an empty schema **/
  Schema() {
    types_ = new CharArray();
    rows_ = new StringArray();
    cols_ = new StringArray();
  }

  /** Create a schema from a string of types. A string that contains
   * characters other than those identifying the four type results in
   * undefined behavior. The argument is external, a nullptr argument is
   * undefined. **/
  Schema(const char *types) {
    types_ = new CharArray();
    rows_ = new StringArray();
    cols_ = new StringArray();

    for (size_t i = 0; i < strlen(types); i++) {
      add_column(types[i], nullptr);
    }
  }

  /** Destructor for Schema.
   * This implementation assume that even though the arguments to a Schema
   * are external, the Schema acquires ownership of everything in its
   * arrays. **/
  ~Schema() {
    for (size_t i = 0; i < width(); i++) {
      delete cols_->get(i);
    }

    for (size_t i = 0; i < length(); i++) {
      delete rows_->get(i);
    }

    delete types_;
    delete cols_;
    delete rows_;
  }

  /** Add a column of the given type and name (can be nullptr), name
   * is external. Names are expectd to be unique, duplicates result
   * in undefined behavior. */
  void add_column(char typ, String *name) {
    types_->push_back(typ);
    cols_->push_back(Util::clone(name));
  }

  /** Add a row with a name (possibly nullptr), name is external.  Names
   * are expectd to be unique, duplicates result in undefined behavior. */
  void add_row(String *name) { rows_->push_back(Util::clone(name)); }

  /** Return name of row at idx; nullptr indicates no name. An idx >=
   * width is undefined. */
  String *row_name(size_t idx) { return rows_->get(idx); }

  /** Return name of column at idx; nullptr indicates no name given.
   *  An idx >= width is undefined.*/
  String *col_name(size_t idx) { return cols_->get(idx); }

  /** Return type of column at idx. An idx >= width is undefined. */
  char col_type(size_t idx) { return types_->get(idx); }

  /** Given a column name return its index, or -1. */
  int col_idx(const char *name) {
    String s(name);
    size_t idx;

    return ((idx = cols_->index_of(&s)) < width()) ? idx : -1;
  }

  /** Given a row name return its index, or -1. */
  int row_idx(const char *name) {
    String s(name);
    size_t idx;

    return ((idx = rows_->index_of(&s)) < width()) ? idx : -1;
  }

  /** The number of columns */
  size_t width() { return cols_->size(); }

  /** The number of rows */
  size_t length() { return rows_->size(); }

  /** A helper function that clears the row names */
  void purge_rows() {
    for (size_t i = 0; i < rows_->size(); i++) {
      delete rows_->get(i);
    }
    rows_->clear();
  }

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
    ser.write(width());
    for (size_t i = 0; i < width(); i++) {
      ser.write(col_type(i));
    }
  }

  /** Deserializes a schema from a deserializer. **/
  static Schema *deserialize(Deserializer &dser) {
    Schema *schema = new Schema();
    size_t width = dser.read_size_t();
    for (size_t i = 0; i < width; i++) {
      schema->add_column(dser.read_char(), nullptr);
    }
    return schema;
  }
};