// lang: CwC
#pragma once

#include "schema.h"

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
  ColumnArray cols_;
  size_t row_idx_;

  /** Build a row following a schema. */
  Row(Schema &scm) {
    row_idx_ = 0;
    scm_ = new Schema(scm);
    for (size_t i = 0; i < scm_->width(); i++) {
      switch (scm_->col_type(i)) {
      case 'B':
        cols_.push_back(new BoolColumn());
        break;
      case 'I':
        cols_.push_back(new IntColumn());
        break;
      case 'F':
        cols_.push_back(new FloatColumn());
        break;
      case 'S':
        cols_.push_back(new StringColumn());
        break;
      }
      cols_.get(i)->push_back_missing();
    }
  }

  /** Destructor for a Row **/
  ~Row() {
    for (size_t i = 0; i < scm_->width(); i++) {
      delete cols_.get(i);
    }
    delete scm_;
  }

  /** Setters: set the given column with the given value. Setting a column
   * with a value of the wrong type is undefined. */
  void set(size_t col, int val) { cols_.get(col)->as_int()->set(0, val); }
  void set(size_t col, float val) { cols_.get(col)->as_float()->set(0, val); }
  void set(size_t col, bool val) { cols_.get(col)->as_bool()->set(0, val); }
  /** Acquire ownership of the string. */
  void set(size_t col, String *val) {
    cols_.get(col)->as_string()->set(0, val);
  }
  /** Sets the given column to missing **/
  void set_missing(size_t col) { cols_.get(col)->set_missing(0); }

  /** Set/get the index of this row (ie. its position in the dataframe.
   * This is only used for informational purposes, unused otherwise */
  void set_idx(size_t idx) { row_idx_ = idx; }
  size_t get_idx() { return row_idx_; }

  /** Getters: get the value at the given column. If the column is not
   * of the requested type, the result is undefined. */
  int get_int(size_t col) { return cols_.get(col)->as_int()->get(0); }
  bool get_bool(size_t col) { return cols_.get(col)->as_bool()->get(0); }
  float get_float(size_t col) { return cols_.get(col)->as_float()->get(0); }
  String *get_string(size_t col) { return cols_.get(col)->as_string()->get(0); }

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