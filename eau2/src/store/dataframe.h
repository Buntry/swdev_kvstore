// lang: CwC
#pragma once

#include "../utils/array.h"
#include "../utils/object.h"
#include "../utils/string.h"
#include "../utils/thread.h"
#include "stdarg.h"

/** Static variable for number of rows before we consider multi-threading. **/
/** This will also be used as a number of when to determine the amount of
 * threads. **/
static const size_t MIN_NROWS_PER_THREAD = 500000;

/** This makes sure we don't go overboard with our number of threads. **/
static const size_t MAX_NUM_THREADS = 8;

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

  /** Returns the number of elements in the column. */
  virtual size_t size() { return 0; }

  /** Return the type of this column as a char: 'S', 'B', 'I' and 'F'. */
  char get_type() { return this->type_; }

  /** Overriding the clone method on object **/
  virtual Column *clone() { return nullptr; }
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

  /** Object methods to satisfy requirement of being an object. **/
  BoolColumn *clone() {
    BoolColumn *bc = new BoolColumn();
    bc->vals_.add_all(0, &vals_);
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

  /** Object methods to satisfy requirement of being an object. **/
  IntColumn *clone() {
    IntColumn *ic = new IntColumn();
    ic->vals_.add_all(0, &vals_);
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

  /** Object methods to satisfy requirement of being an object. **/
  FloatColumn *clone() {
    FloatColumn *fc = new FloatColumn();
    fc->vals_.add_all(0, &vals_);
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
  virtual void push_back(String *s) { vals_.push_back(Util::clone(s)); };

  /** Object methods to satisfy requirement of being an object. **/
  StringColumn *clone() {
    StringColumn *sc = new StringColumn();
    for (size_t i = 0; i < size(); i++) {
      sc->push_back(get(i));
    }
    return sc;
  }
};

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
};

/*****************************************************************************
 * Fielder::
 * A field vistor invoked by Row.
 *
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu
 */
class Fielder : public Object {
public:
  /** Called before visiting a row, the argument is the row offset in the
    dataframe. */
  virtual void start(size_t r) {}

  /** Called for fields of the argument's type with the value of the
     field. */
  virtual void accept(bool b) {}
  virtual void accept(float f) {}
  virtual void accept(int i) {}
  virtual void accept(String *s) {}

  /** Called when all fields have been seen. */
  virtual void done() {}
};

/*************************************************************************
 * Row::
 *
 * This class represents a single row of data constructed according to a
 * dataframe's schema. The purpose of this class is to make it easier to add
 * read/write complete rows. Internally a dataframe hold data in columns.
 * Rows have pointer equality.
 *
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu
 */
class Row : public Object {
public:
  Schema *scm_;
  Column **cols_;
  size_t row_idx_;

  /** Build a row following a schema. */
  Row(Schema &scm) {
    row_idx_ = 0;
    scm_ = new Schema(scm);
    cols_ = new Column *[scm_->width()];
    for (size_t i = 0; i < scm_->width(); i++) {
      switch (scm_->col_type(i)) {
      case 'B':
        cols_[i] = new BoolColumn();
        break;
      case 'I':
        cols_[i] = new IntColumn();
        break;
      case 'F':
        cols_[i] = new FloatColumn();
        break;
      case 'S':
        cols_[i] = new StringColumn();
        break;
      }
    }
  }

  /** Destructor for a Row **/
  ~Row() {
    for (size_t i = 0; i < scm_->width(); i++) {
      delete cols_[i];
    }
    delete[] cols_;
    delete scm_;
  }

  /** Setters: set the given column with the given value. Setting a column
   * with a value of the wrong type is undefined. */
  void set(size_t col, int val) {
    if (cols_[col]->size() == 0) {
      cols_[col]->as_int()->push_back(0);
    }
    cols_[col]->as_int()->set(0, val);
  }
  void set(size_t col, float val) {
    if (cols_[col]->size() == 0) {
      cols_[col]->as_float()->push_back(0.0f);
    }
    cols_[col]->as_float()->set(0, val);
  }
  void set(size_t col, bool val) {
    if (cols_[col]->size() == 0) {
      cols_[col]->as_bool()->push_back(false);
    }
    cols_[col]->as_bool()->set(0, val);
  }
  /** Acquire ownership of the string. */
  void set(size_t col, String *val) {
    if (cols_[col]->size() == 0) {
      cols_[col]->push_back(nullptr);
    }
    cols_[col]->as_string()->set(0, val);
  }

  /** Set/get the index of this row (ie. its position in the dataframe.
   * This is only used for informational purposes, unused otherwise */
  void set_idx(size_t idx) { row_idx_ = idx; }
  size_t get_idx() { return row_idx_; }

  /** Getters: get the value at the given column. If the column is not
   * of the requested type, the result is undefined. */
  int get_int(size_t col) { return cols_[col]->as_int()->get(0); }
  bool get_bool(size_t col) { return cols_[col]->as_bool()->get(0); }
  float get_float(size_t col) { return cols_[col]->as_float()->get(0); }
  String *get_string(size_t col) { return cols_[col]->as_string()->get(0); }

  /** Number of fields in the row. */
  size_t width() { return scm_->width(); }

  /** Type of the field at the given position. An idx >= width is
     undefined. */
  char col_type(size_t idx) { return scm_->col_type(idx); }

  /** Given a Fielder, visit every field of this row. The first argument
   * is index of the row in the dataframe.
   * Calling this method before the row's fields have been set is
   * undefined. */
  void visit(size_t idx, Fielder &f) {
    assert(idx == get_idx());
    f.start(idx);
    for (size_t i = 0; i < width(); i++) {
      switch (col_type(i)) {
      case 'B':
        f.accept(get_bool(i));
        break;
      case 'I':
        f.accept(get_int(i));
        break;
      case 'F':
        f.accept(get_float(i));
        break;
      case 'S':
        f.accept(get_string(i));
        break;
      default:
        assert(false);
      }
    }
    f.done();
  }
};

/*******************************************************************************
 *  Rower::
 *  An interface for iterating through each row of a data frame. The intent
 *  is that this class should subclassed and the accept() method be given
 *  a meaningful implementation. Rowers can be cloned for parallel execution.
 *
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu
 */
class Rower : public Object {
public:
  /** This method is called once per row. The row object is on loan and
      should not be retained as it is likely going to be reused in the next
      call. The return value is used in filters to indicate that a row
      should be kept. */
  virtual bool accept(Row &r) { return false; }

  /** Once traversal of the data frame is complete the rowers that were
      split off will be joined.  There will be one join per split. The
      original object will be the last to be called join on. The join
     method is reponsible for cleaning up memory. */
  virtual void join_delete(Rower *other) { delete other; }

  /** Satisifies Object properties **/
  virtual Rower *clone() { return nullptr; }
};

/*******************************************************************************
 * PrintFielder::
 *
 * Prints fields in SoR format without new lines.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu
 */
class PrintFielder : public Fielder {
public:
  /** Prints the corresponding value surrounded with angle brackets. **/
  void accept(bool b) { p("<").p(b).p(">"); }
  void accept(float f) { p("<").p(f).p(">"); }
  void accept(int i) { p("<").p(i).p(">"); }
  void accept(String *s) {
    p("<\"").p((s == nullptr) ? "" : s->c_str()).p("\">");
  }

  /** Once we're done with the file, print a new line. **/
  void done() { pln(); }
};

/*******************************************************************************
 * PrintRower::
 *
 * Prints rows in an SoR file format to standard output.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu
 */
class PrintRower : public Rower {
public:
  PrintFielder pf;
  bool accept(Row &r) {
    r.visit(r.get_idx(), pf);
    return true;
  }
};

/** Forward declaration of DataFrame to allow for compilation **/
class DataFrame;

/*******************************************************************************
 * RowThread::
 *
 * A RowThread asynchronously applies a given Rower to a range of Rows in a
 * DataFrame.
 *
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu
 */
class RowThread : public Thread {
public:
  DataFrame *df_;
  Rower *rower_;
  size_t start_, end_;

  /** Initializes a RowThread. Nullptr DataFrame and Rowers are undefined
   * behavior. **/
  RowThread(DataFrame *df, Rower *rower, size_t start, size_t end)
      : df_(df), rower_(rower), start_(start), end_(end) {
    assert(start < end);
  }

  void run();
};

/****************************************************************************
 * DataFrame::
 *
 * A DataFrame is table composed of columns of equal length. Each column
 * holds values of the same type (I, S, B, F). A dataframe has a schema that
 * describes it.
 */
class DataFrame : public Object {
public:
  Schema *scm_;
  Column **cols_;

  /** Create a data frame with the same columns as the give df but no rows
   * or row names. */
  DataFrame(DataFrame &df) : DataFrame(*df.scm_) {}

  /** Create a data frame from a schema. */
  DataFrame(Schema &schema) {
    scm_ = new Schema(schema);
    scm_->purge_rows();
    cols_ = new Column *[scm_->width()];
    for (size_t i = 0; i < scm_->width(); i++) {
      switch (scm_->col_type(i)) {
      case 'B':
        cols_[i] = new BoolColumn();
        break;
      case 'I':
        cols_[i] = new IntColumn();
        break;
      case 'F':
        cols_[i] = new FloatColumn();
        break;
      case 'S':
        cols_[i] = new StringColumn();
        break;
      }
    }
  }

  /** Destructor for a DataFrame **/
  ~DataFrame() {
    for (size_t i = 0; i < ncols(); i++) {
      delete cols_[i];
    }
    delete[] cols_;
    delete scm_;
  }

  /** Returns the dataframe's schema. Modifying the schema after a
   * dataframe has been created in undefined. */
  Schema &get_schema() { return *scm_; }

  /** Adds a column this dataframe, updates the schema, the new column
   * is external, and appears as the last column of the dataframe, the
   * name is optional and external. A nullptr colum is undefined. */
  void add_column(Column *col, String *name) {
    Column **new_cols = new Column *[ncols() + 1];
    for (size_t i = 0; i < ncols(); i++) {
      new_cols[i] = cols_[i];
    }
    new_cols[ncols()] = col->clone();
    // If this is our first column, grow the schema's rows to fit it.
    if (ncols() == 0) {
      for (size_t i = 0; i < col->size(); i++) {
        scm_->add_row(nullptr);
      }
    }
    scm_->add_column(col->get_type(), name);
    delete[] cols_;
    cols_ = new_cols;
  }

  /** Return the value at the given column and row. Accessing rows or
   *  columns out of bounds, or request the wrong type is undefined.*/
  int get_int(size_t col, size_t row) { return cols_[col]->as_int()->get(row); }
  bool get_bool(size_t col, size_t row) {
    return cols_[col]->as_bool()->get(row);
  }
  float get_float(size_t col, size_t row) {
    return cols_[col]->as_float()->get(row);
  }
  String *get_string(size_t col, size_t row) {
    return cols_[col]->as_string()->get(row);
  }

  /** Return the offset of the given column name or -1 if no such col. */
  int get_col(String &col) { return scm_->col_idx(col.c_str()); }

  /** Return the offset of the given row name or -1 if no such row. */
  int get_row(String &row) { return scm_->row_idx(row.c_str()); }

  /** Set the value at the given column and row to the given value.
   * If the column is not  of the right type or the indices are out of
   * bound, the result is undefined. */
  void set(size_t col, size_t row, int val) {
    cols_[col]->as_int()->set(row, val);
  }
  void set(size_t col, size_t row, bool val) {
    cols_[col]->as_bool()->set(row, val);
  }
  void set(size_t col, size_t row, float val) {
    cols_[col]->as_float()->set(row, val);
  }
  void set(size_t col, size_t row, String *val) {
    cols_[col]->as_string()->set(row, val);
  }

  /** Set the fields of the given row object with values from the columns
   * at the given offset.  If the row is not form the same schema as the
   * dataframe, results are undefined.
   */
  void fill_row(size_t idx, Row &row) {
    row.set_idx(idx);
    for (size_t i = 0; i < ncols(); i++) {
      String *s;
      switch (scm_->col_type(i)) {
      case 'B':
        row.set(i, get_bool(i, idx));
        break;
      case 'I':
        row.set(i, get_int(i, idx));
        break;
      case 'F':
        row.set(i, get_float(i, idx));
        break;
      case 'S':
        s = get_string(i, idx);
        row.set(i, (s == nullptr) ? s : s->clone());
        break;
      default:
        assert(false);
        break;
      }
    }
  }

  /** Add a row at the end of this dataframe. The row is expected to have
   *  the right schema and be filled with values, otherwise undefined.  */
  void add_row(Row &row) {
    for (size_t i = 0; i < ncols(); i++) {
      switch (scm_->col_type(i)) {
      case 'B':
        cols_[i]->push_back(row.get_bool(i));
        break;
      case 'I':
        cols_[i]->push_back(row.get_int(i));
        break;
      case 'F':
        cols_[i]->push_back(row.get_float(i));
        break;
      case 'S':
        cols_[i]->push_back(row.get_string(i));
        break;
      default:
        assert(false);
      }
    }

    scm_->add_row(nullptr);
  }

  /** The number of rows in the dataframe. */
  size_t nrows() { return scm_->length(); }

  /** The number of columns in the dataframe.*/
  size_t ncols() { return scm_->width(); }

  /** Visit rows in order */
  void map(Rower &r) {
    Row row(*scm_);
    for (size_t i = 0; i < nrows(); i++) {
      fill_row(i, row);
      r.accept(row);
    }
  }

  /** Create a new dataframe, constructed from rows for which the given
   * Rower returned true from its accept method. */
  DataFrame *filter(Rower &r) {
    DataFrame *df = new DataFrame(*scm_);
    Row row(*scm_);
    for (size_t i = 0; i < nrows(); i++) {
      fill_row(i, row);
      if (r.accept(row)) {
        df->add_row(row);
      }
    }
    return df;
  }

  /** Print the dataframe in SoR format to standard output. */
  void print() {
    PrintRower pr;
    map(pr);
  }

  /** This method clones the Rower and executes the map in parallel. Join is
   * used at the end to merge the results. */
  void pmap(Rower &r) {
    // Default to one thread.
    size_t num_threads = 1;
    size_t rows_per_thread = (nrows() / num_threads) + 1;

    // While our rows per thread is greater than double our minimum
    while (rows_per_thread >= MIN_NROWS_PER_THREAD * 2 &&
           num_threads < MAX_NUM_THREADS) {
      num_threads++;
      rows_per_thread = (nrows() / num_threads) + 1;
    }

    // If we aren't big enough to warrant multiple threads, defer to map.
    if (num_threads == 1) {
      return map(r);
    }

    RowThread **row_threads = new RowThread *[num_threads];
    Rower **rowers = new Rower *[num_threads];

    // Spawn Threads (inefficient, but will do for now)
    for (size_t i = 0; i < num_threads; i++) {
      // Select start and end as a maximum of chunk sizes
      size_t start = rows_per_thread * i;
      size_t chunk_end = (i + 1) * rows_per_thread;
      size_t end = (nrows() <= chunk_end) ? nrows() : chunk_end;

      // Clone the given rower and start all of the threads
      rowers[i] = (i == 0) ? &r : rowers[i - 1]->clone();
      row_threads[i] = new RowThread(this, rowers[i], start, end);
      row_threads[i]->start();
    }

    // Assure all threads are finished
    for (size_t i = 0; i < num_threads; i++) {
      row_threads[i]->join();
      delete row_threads[i];
    }
    delete[] row_threads;

    // Join delete the rowers to reduce the result.
    for (size_t i = num_threads - 1; i > 0; i--) {
      rowers[i - 1]->join_delete(rowers[i]);
    }
    delete[] rowers;
  }
};

/** Applies this RowThread's rower on a range of Rows
 * of a DataFrame. **/
void RowThread::run() {
  Row row(df_->get_schema());
  for (size_t i = start_; i < end_; i++) {
    df_->fill_row(i, row);
    rower_->accept(row);
  }
}
