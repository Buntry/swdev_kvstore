// lang: CwC
#pragma once

#include "../utils/array.h"
#include "stdarg.h"

/** Forward declarations to make Column Class compile. **/
class BoolColumn;
class IntColumn;
class FloatColumn;
class StringColumn;

/**************************************************************************
 * Column ::
 * Represents one column of a data frame which holds values of a single type.
 * This abstract class defines methods overriden in subclasses. There is
 * one subclass per element type. Columns are mutable, equality is pointer
 * equality.
 *
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.
 * */
class Column : public Object {
public:
  char type_ = '\0';
  BoolArray missing_;

  /** Type converters: Return same column under its actual type, or
   *  nullptr if of the wrong type.  */
  virtual IntColumn *as_int() { return nullptr; }
  virtual BoolColumn *as_bool() { return nullptr; }
  virtual FloatColumn *as_float() { return nullptr; }
  virtual StringColumn *as_string() { return nullptr; }

  /** Type appropriate push_back methods. Calling the wrong method is
   * undefined behavior. **/
  virtual void push_back(int val) { assert(false); };
  virtual void push_back(bool val) { assert(false); };
  virtual void push_back(float val) { assert(false); };
  virtual void push_back(String *val) { assert(false); };

  /** Deals with allowing missing values. **/
  virtual void push_back_missing() { missing_.push_back(true); }
  bool is_missing(size_t i) { return missing_.get(i); }

  /** Returns the number of elements in the column. */
  virtual size_t size() { return missing_.size(); }

  /** Return the type of this column as a char: 'S', 'B', 'I' and 'F'. */
  char get_type() { return this->type_; }

  /** Overriding the clone method on object **/
  virtual Column *clone() { return nullptr; }

  /** Serializes this column onto the given serializer object. **/
  virtual void serialize(Serializer &ser) {
    ser.write(type_);
    ser.write(size());
    for (size_t i = 0; i < size(); i++) {
      ser.write(is_missing(i));
    }
  }

  /** Deserializes a column of the correct type. **/
  static Column *deserialize(Deserializer &dser);
};

/*************************************************************************
 * BoolColumn::
 * Holds primitive bool values, unwrapped.
 *
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu
 */
class BoolColumn : public Column {
public:
  BoolArray vals_;

  BoolColumn() { type_ = 'B'; }

  BoolColumn(int n, ...) : BoolColumn() {
    int i;
    bool val;
    va_list vl;

    va_start(vl, n);
    for (i = 0; i < n; i++) {
      val = va_arg(vl, int);
      push_back(val);
    }
    va_end(vl);
  }

  bool get(size_t idx) { return vals_.get(idx); }
  BoolColumn *as_bool() { return this; }
  /** Set value at idx. An out of bound idx is undefined.  */
  void set(size_t idx, bool val) { vals_.set(idx, val); }
  size_t size() { return vals_.size(); }
  void push_back(bool val) { vals_.push_back(val); };

  /** Push back a dummy on missing **/
  void push_back_missing() {
    Column::push_back_missing();
    vals_.push_back(0);
  }

  /** Object methods to satisfy requirement of being an object. **/
  BoolColumn *clone() {
    BoolColumn *bc = new BoolColumn();
    for (size_t i = 0; i < size(); i++)
      bc->push_back(get(i));
    return bc;
  }

  /** Serializes this column onto the given serializer object. **/
  virtual void serialize(Serializer &ser) {
    Column::serialize(ser);
    for (size_t i = 0; i < size(); i++)
      ser.write(get(i));
  }

  /** Deserializes a BoolColumn from the given deserializer object. **/
  static BoolColumn *deserialize(Deserializer &dser) {
    BoolColumn *bc = new BoolColumn();
    size_t len = dser.read_size_t();
    BoolArray missing;
    for (size_t i = 0; i < len; i++) {
      missing.push_back(dser.read_bool());
    }
    for (size_t i = 0; i < len; i++) {
      if (missing.get(i)) {
        bc->push_back_missing();
      } else {
        bc->push_back(dser.read_bool());
      }
    }
    return bc;
  }
};

/*************************************************************************
 * IntColumn::
 * Holds primitive int values, unwrapped.
 *
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu
 */
class IntColumn : public Column {
public:
  IntArray vals_;

  IntColumn() { type_ = 'I'; }

  IntColumn(int n, ...) : IntColumn() {
    int i;
    int val;
    va_list vl;

    va_start(vl, n);
    for (i = 0; i < n; i++) {
      val = va_arg(vl, int);
      push_back(val);
    }
    va_end(vl);
  }

  int get(size_t idx) { return vals_.get(idx); }
  IntColumn *as_int() { return this; }
  /** Set value at idx. An out of bound idx is undefined.  */
  void set(size_t idx, int val) { vals_.set(idx, val); }
  size_t size() { return vals_.size(); }
  void push_back(int val) { vals_.push_back(val); };

  /** Push back a dummy on missing **/
  void push_back_missing() {
    Column::push_back_missing();
    vals_.push_back(0);
  }

  /** Object methods to satisfy requirement of being an object. **/
  IntColumn *clone() {
    IntColumn *ic = new IntColumn();
    for (size_t i = 0; i < size(); i++)
      ic->push_back(get(i));
    return ic;
  }

  /** Serializes this column onto the given serializer object. **/
  virtual void serialize(Serializer &ser) {
    Column::serialize(ser);
    for (size_t i = 0; i < size(); i++)
      ser.write(get(i));
  }

  /** Deserializes a IntColumn from the given deserializer object. **/
  static IntColumn *deserialize(Deserializer &dser) {
    IntColumn *ic = new IntColumn();
    size_t len = dser.read_size_t();
    BoolArray missing;
    for (size_t i = 0; i < len; i++) {
      missing.push_back(dser.read_bool());
    }
    for (size_t i = 0; i < len; i++) {
      if (missing.get(i)) {
        ic->push_back_missing();
      } else {
        ic->push_back(dser.read_int());
      }
    }
    return ic;
  }
};

/*************************************************************************
 * FloatColumn::
 * Holds primitive int values, unwrapped.
 *
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu
 */
class FloatColumn : public Column {
public:
  FloatArray vals_;

  FloatColumn() { type_ = 'F'; }

  /** A variable argument constructor for passing any number of floats. **/
  FloatColumn(int n, ...) : FloatColumn() {
    int i;
    float val;
    va_list vl;

    va_start(vl, n);
    for (i = 0; i < n; i++) {
      val = va_arg(vl, double);
      push_back(val);
    }
    va_end(vl);
  }

  float get(size_t idx) { return vals_.get(idx); }
  FloatColumn *as_float() { return this; }
  /** Set value at idx. An out of bound idx is undefined.  */
  void set(size_t idx, float val) { vals_.set(idx, val); }
  size_t size() { return vals_.size(); }
  void push_back(float val) { vals_.push_back(val); };

  /** Push back a dummy on missing **/
  void push_back_missing() {
    Column::push_back_missing();
    vals_.push_back(0);
  }

  /** Object methods to satisfy requirement of being an object. **/
  FloatColumn *clone() {
    FloatColumn *fc = new FloatColumn();
    for (size_t i = 0; i < size(); i++)
      fc->push_back(get(i));
    return fc;
  }

  /** Serializes this column onto the given serializer object. **/
  virtual void serialize(Serializer &ser) {
    Column::serialize(ser);
    for (size_t i = 0; i < size(); i++)
      ser.write(get(i));
  }

  /** Deserializes a FloatColumn from the given deserializer object. **/
  static FloatColumn *deserialize(Deserializer &dser) {
    FloatColumn *fc = new FloatColumn();
    size_t len = dser.read_size_t();
    BoolArray missing;
    for (size_t i = 0; i < len; i++) {
      missing.push_back(dser.read_bool());
    }
    for (size_t i = 0; i < len; i++) {
      if (missing.get(i)) {
        fc->push_back_missing();
      } else {
        fc->push_back(dser.read_float());
      }
    }
    return fc;
  }
};

/*************************************************************************
 * StringColumn::
 * Holds string pointers. The strings are external.  Nullptr is a valid
 * value.
 *
 * A StringColumn owns all of the Strings inside of it. This means that
 * we clone Strings if they've been passed to an external-accepting
 * function.
 *
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu
 */
class StringColumn : public Column {
public:
  StringArray vals_;

  StringColumn() { type_ = 'S'; }

  /** This constructor clones its arguments.  **/
  StringColumn(int n, ...) : StringColumn() {
    int i;
    String *val;
    va_list vl;

    va_start(vl, n);
    for (i = 0; i < n; i++) {
      val = va_arg(vl, String *);
      push_back(val);
    }
    va_end(vl);
  }

  /** A String has ownership over all of its contents. Therefore,
   * before we delete it, we need to delete all the Strings it contains. **/
  ~StringColumn() {
    for (size_t i = 0; i < vals_.size(); i++) {
      delete vals_.get(i);
    }
  }

  /** Casts this column as itself **/
  StringColumn *as_string() { return this; }

  /** Returns the string at idx; undefined on invalid idx.*/
  String *get(size_t idx) { return vals_.get(idx); }

  /** Acquire ownership of the string. Out of bound idx is undefined.
   * Since a StrColumn clones */
  void set(size_t idx, String *val) { delete vals_.set(idx, val); }
  size_t size() { return vals_.size(); }

  /** Since Strings are external unless otherwise stated, we make a clone
   * so that we can manage its memory. **/
  virtual void push_back(String *s) {
    if (s == nullptr) {
      push_back_missing();
    } else {
      vals_.push_back(s->clone());
    }
  };

  /** Pushes back, but steals the value. Used in deserialization **/
  virtual void push_back_steal_(String *s) {
    assert(s != nullptr);
    vals_.push_back(s);
  }

  /** Push back a dummy on missing **/
  void push_back_missing() {
    Column::push_back_missing();
    vals_.push_back(nullptr);
  }

  /** Object methods to satisfy requirement of being an object. **/
  StringColumn *clone() {
    StringColumn *sc = new StringColumn();
    for (size_t i = 0; i < size(); i++) {
      sc->push_back(get(i));
    }
    return sc;
  }

  /** Serializes this column onto the given serializer object. **/
  virtual void serialize(Serializer &ser) {
    Column::serialize(ser);
    for (size_t i = 0; i < size(); i++)
      ser.write(get(i));
  }

  /** Deserializes a StringColumn from the given deserializer object. **/
  static StringColumn *deserialize(Deserializer &dser) {
    StringColumn *sc = new StringColumn();
    size_t len = dser.read_size_t();
    BoolArray missing;
    for (size_t i = 0; i < len; i++) {
      missing.push_back(dser.read_bool());
    }
    for (size_t i = 0; i < len; i++) {
      if (missing.get(i)) {
        sc->push_back_missing();
      } else {
        sc->push_back_steal_(String::deserialize(dser));
      }
    }
    return sc;
  }
};

/** Deserializes a column of the correct type. **/
static Column *deserialize(Deserializer &dser) {
  char type = dser.read_char();
  switch (type) {
  case 'B':
    return BoolColumn::deserialize(dser);
  case 'I':
    return IntColumn::deserialize(dser);
  case 'F':
    return FloatColumn::deserialize(dser);
  case 'S':
    return StringColumn::deserialize(dser);
  default:
    assert(false);
    return nullptr;
  }
}

/** Generate column array class **/
generate_object_classarray(ColumnArray, Column);