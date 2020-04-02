// lang: CwC
#pragma once

#include "../utils/thread.h"
#include "kvstore-fd.h"
#include "rows.h"

/** Static variable for number of rows before we consider multi-threading. **/
/** This will also be used as a number of when to determine the amount of
 * threads. **/
static const size_t MIN_NROWS_PER_THREAD = 500000;

/** This makes sure we don't go overboard with our number of threads. **/
static const size_t MAX_NUM_THREADS = 8;

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
  ColumnArray cols_;

  /** Create a data frame with the same columns as the give df but no rows
   * or row names. */
  DataFrame(DataFrame &df) : DataFrame(*df.scm_) {}

  /** Create a data frame from a schema. */
  DataFrame(Schema &schema) {
    scm_ = new Schema(schema);
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
    }
  }

  /** Create a dataframe from a schema and KVStore. **/
  DataFrame(Schema &schema, KVStore *store) : DataFrame(schema) {
    store_ = store;
  }

  /** Destructor for a DataFrame **/
  ~DataFrame() {
    for (size_t i = 0; i < ncols(); i++) {
      delete cols_.get(i);
    }
    delete scm_;
    delete key_;
    delete dist_scm_;
  }

  /** Returns the dataframe's schema. Modifying the schema after a
   * dataframe has been created in undefined. */
  Schema &get_schema() { return *scm_; }

  /** Adds a column this dataframe, updates the schema, the new column
   * is external, and appears as the last column of the dataframe, the
   * name is optional and external. A nullptr colum is undefined. */
  void add_column(Column *col) {
    // If this is our first column, grow the schema's rows to fit it.
    if (ncols() == 0) {
      for (size_t i = 0; i < col->size(); i++) {
        scm_->add_row();
      }
    }
    scm_->add_column(col->get_type());
    cols_.push_back(col->clone());
  }

  /** Return the value at the given column and row. Accessing rows or
   *  columns out of bounds, or request the wrong type is undefined.*/
  int local_get_int(size_t col, size_t row) {
    return cols_.get(col)->as_int()->get(row);
  }
  bool local_get_bool(size_t col, size_t row) {
    return cols_.get(col)->as_bool()->get(row);
  }
  float local_get_float(size_t col, size_t row) {
    return cols_.get(col)->as_float()->get(row);
  }
  String *local_get_string(size_t col, size_t row) {
    return cols_.get(col)->as_string()->get(row);
  }

  /** Set the value at the given column and row to the given value.
   * If the column is not  of the right type or the indices are out of
   * bound, the result is undefined. */
  void set(size_t col, size_t row, int val) {
    cols_.get(col)->as_int()->set(row, val);
  }
  void set(size_t col, size_t row, bool val) {
    cols_.get(col)->as_bool()->set(row, val);
  }
  void set(size_t col, size_t row, float val) {
    cols_.get(col)->as_float()->set(row, val);
  }
  void set(size_t col, size_t row, String *val) {
    cols_.get(col)->as_string()->set(row, val);
  }

  /** Determines if the given value is missing. **/
  bool is_missing(size_t col, size_t row) {
    return cols_.get(col)->is_missing(row);
  }

  /** Set the fields of the given row object with values from the columns
   * at the given offset.  If the row is not form the same schema as the
   * dataframe, results are undefined.
   */
  void fill_row(size_t idx, Row &row) {
    row.set_idx(idx);
    for (size_t i = 0; i < ncols(); i++) {
      if (is_missing(i, idx)) {
        row.set_missing(i);
        continue;
      }

      switch (scm_->col_type(i)) {
      case 'B':
        row.set(i, local_get_bool(i, idx));
        break;
      case 'I':
        row.set(i, local_get_int(i, idx));
        break;
      case 'F':
        row.set(i, local_get_float(i, idx));
        break;
      case 'S':
        row.set(i, local_get_string(i, idx)->clone());
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
        cols_.get(i)->push_back(row.get_bool(i));
        break;
      case 'I':
        cols_.get(i)->push_back(row.get_int(i));
        break;
      case 'F':
        cols_.get(i)->push_back(row.get_float(i));
        break;
      case 'S':
        cols_.get(i)->push_back(row.get_string(i));
        break;
      default:
        assert(false);
      }
    }

    scm_->add_row();
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

  /** Equality for Dataframes is based on values and not names. **/
  bool equals(Object *o) {
    DataFrame *that = dynamic_cast<DataFrame *>(o);
    if (that == nullptr || that->nrows() != nrows() || that->ncols() != ncols())
      return false;
    for (size_t i = 0; i < ncols(); i++)
      if (!cols_.get(i)->equals(that->cols_.get(i)))
        return false;
    return true;
  }

  /** Hash function for dataframe. **/
  size_t hash() { return nrows() ^ ncols(); }

  /** Clones this dataframe. **/
  DataFrame *clone() {
    Schema s;
    DataFrame *df = new DataFrame(s);
    for (size_t i = 0; i < ncols(); i++) {
      df->add_column(cols_.get(i));
    }
    return df;
  }

  /***************************************************************************
   *  BENEATH THIS ARE ALL IMPLEMENTATION FOR A DISTRIBUTED DATAFRAME
   * *************************************************************************/
  KVStore *store_ = nullptr;
  Key *key_ = nullptr;
  Schema *dist_scm_ = nullptr;
  bool is_distributed = false;
  bool must_load_ = false;

  /** Adds a distributed schema to this DataFrame, consuming the schema. This
   * schema contains the true dimensions of the data, as well as which chunks
   * are locally stored. **/
  void set_distributed_schema_(Key *key, Schema *distributed_schema) {
    key_ = key->clone();
    dist_scm_ = distributed_schema;
    is_distributed = true;
  }

  /** Enforces that the **/
  void must_load_on_next_query_() { must_load_ = true; }

  /** Returns the current chunk offset for the given column. **/
  size_t chunk_offset_(size_t col) { return dist_scm_->chunk_index(col); }

  /** Loads data if it is not currently stored in local storage. Returns whether
   * or not the data loaded. **/
  bool load_if_necessary_(size_t col, size_t row) {
    if (!is_distributed)
      return false;
    size_t desired_chunk = row / CHUNK_SIZE;
    if (!must_load_ && chunk_offset_(col) == desired_chunk) {
      return false;
    }
    must_load_ = false;

    // Set up key parameters
    StrBuff sb;
    sb.c(*key_->key()).c("-column").c(col).c("-chunk").c(desired_chunk);
    size_t target = (key_->node() + desired_chunk) % arg.num_nodes;

    // Determine if we should grab this data from another node.
    Key *chunk_key = new Key(sb.get(), target);
    Value *val = (target == store_->index())
                     ? store_->get_value(chunk_key)->clone()
                     : store_->get_and_wait_value(chunk_key);
    Deserializer dser(*val->blob());
    delete chunk_key;
    delete val;
    delete cols_.set(col, Column::deserialize(dser));
    dist_scm_->chunk_indexes_->set(col, desired_chunk);
    return true;
  }

  /** Normalizes a row index to the current chunk offset for that column. **/
  size_t normalize_(size_t col, size_t row) {
    return row - (chunk_offset_(col) * CHUNK_SIZE);
  }

  /** Gets a float from this distributed dataframe. **/
  float get_float(size_t col, size_t row) {
    load_if_necessary_(col, row);
    return local_get_float(col, normalize_(col, row));
  }

  /** Distributes an n-by-1 dataframe across the network. Following a protocol:
   * 1. The schema containing the dimension of the array is stored at the given
   * Key.
   * 2. Chunks of the column are stored starting at the first key, wrapping
   * around in ascending index order. **/
  static DataFrame *fromArray(Key *key, KVStore *kv, size_t sz, float *vals) {
    Schema *distributed_schema = new Schema("F");

    Schema mt;
    DataFrame *df = new DataFrame(mt, kv);

    // Get the number of chunks to split the data into
    size_t rem = sz % CHUNK_SIZE;
    size_t num_chunks = (sz / CHUNK_SIZE) + 1;

    for (size_t c = 0; c < num_chunks; c++) {
      FloatColumn fc;
      size_t limit = (c == num_chunks - 1) ? rem : CHUNK_SIZE;
      for (size_t i = 0; i < limit; i++) {
        fc.push_back(vals[c * CHUNK_SIZE + i]);
        distributed_schema->add_row();
      }

      if (c == 0) {
        df->add_column(&fc);
      }

      // Serialize and store this chunk of the column.
      Serializer ser;
      fc.serialize(ser);
      Value *value = new Value(*ser.data());

      StrBuff sb;
      sb.c(*key->key()).c("-column0-chunk").c(c);

      Key *chunk_key = new Key(sb.get(), (key->node() + c) % arg.num_nodes);
      kv->put(chunk_key, value);
      delete chunk_key;
    }

    Serializer ser;
    distributed_schema->serialize(ser);
    kv->put(key, new Value(*ser.data()));

    // Let the df know about its true schema
    df->set_distributed_schema_(key, distributed_schema);
    return df;
  }

  /** Stores a 1-by-1 dataframe at the given node in the KVStore. **/
  static DataFrame *fromScalar(Key *key, KVStore *kv, float value) {
    Schema *distributed_schema = new Schema("F");

    FloatColumn fc(1, value);
    distributed_schema->add_row();
    Schema mt;

    DataFrame *df = new DataFrame(mt, kv);
    df->add_column(&fc);
    df->set_distributed_schema_(key, distributed_schema);

    // Add the chunk with one value.
    StrBuff sb;
    sb.c(*key->key()).c("-column0-chunk0");
    Serializer fc_ser;
    fc.serialize(fc_ser);

    Key *chunk_key = new Key(sb.get(), key->node());
    kv->put(chunk_key, new Value(*fc_ser.data()));
    delete chunk_key;

    // Also put the distributed schema value.
    Serializer ser;
    distributed_schema->serialize(ser);
    kv->put(key, new Value(*ser.data()));

    return df;
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
