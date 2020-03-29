#include "../src/store/dataframe.h"
#include "test-macros.h"
#include <gtest/gtest.h>

/**
 * @brief Here's are our unit tests for dataframes.
 *
 * @author griep.p@husky.neu.edu, colabella.a@husky.neu.edu
 */
class DataFrameTest : public ::testing::Test {
public:
  /** Schema **/
  Schema *s1;
  Schema *s2;

  /** Row **/

  /** Col **/
  FloatColumn *nefc;
  BoolColumn *nebc;
  size_t initial_size;

  /** Strings **/
  String *a;
  String *b;
  String *c;

  /** DataFrame **/
  DataFrame *d;

  /** Initializes everything **/
  void SetUp() {
    a = new String("test1");
    b = new String("test2");
    c = new String("test3");
    s1 = new Schema("BF");
    s2 = new Schema("BF");
    initial_size = 3;
    nebc = new BoolColumn(initial_size, true, false, true);
    nefc = new FloatColumn(initial_size, 1.02f, 2.02f, 3.02f);
    d = new DataFrame(*s1);
  }

  /** Tears everything down **/
  void TearDown() {
    delete s1;
    delete s2;
    delete a;
    delete b;
    delete c;
    delete nefc;
    delete nebc;
    delete d;
  }
};

TEST_F(DataFrameTest, Loads) {}

TEST_F(DataFrameTest, GetSchema) { ASSERT(d->get_schema().equals(s2)); }

TEST_F(DataFrameTest, EmptyRowsCols) {
  ASSERT_EQ(d->nrows(), 0);
  ASSERT_EQ(d->ncols(), 2);
}

TEST_F(DataFrameTest, Cols) {
  Schema s;
  DataFrame df(s);

  df.add_column(nebc);
  df.add_column(nebc);

  ASSERT_EQ(df.ncols(), 2);
  ASSERT_EQ(df.nrows(), 3);
}

TEST_F(DataFrameTest, MultipleColumnsAndNewRow) {
  // An empty Schema
  Schema s;

  // A new Schema for a new Row
  Schema s2("IFBS");

  // Create a DataFrame with the Schema
  DataFrame df(s);

  // Strings to add to StringColumn
  String *a = new String("hello world");
  String *q = new String("quick brown fox jumped");

  // Strings for column names
  String *b = new String("int");
  String *c = new String("float");
  String *d = new String("bool");
  String *e = new String("string");

  // Columns to add to DataFrame
  IntColumn *ic = new IntColumn(4, 1, 2, 3, 4);
  FloatColumn *fc = new FloatColumn(4, 0.1f, 0.1f, 0.1f, 0.1f);
  BoolColumn *bc = new BoolColumn(4, true, false, false, true);
  StringColumn *sc = new StringColumn(4, a, a, a, a);

  // Add to DataFrame
  df.add_column(ic);
  df.add_column(fc);
  df.add_column(bc);
  df.add_column(sc);

  // Check DataFrame dimensions
  ASSERT_EQ(df.ncols(), 4);
  ASSERT_EQ(df.nrows(), 4);

  // New row
  Row r(s2);

  r.set(0, 5);
  r.set(1, 0.2f);
  r.set(2, true);
  r.set(3, q); // this row acquires ownership of q.

  df.add_row(r);

  // Check that you can get column values
  ASSERT_EQ(df.local_get_int(0, 4), 5);
  ASSERT_EQ(df.local_get_float(1, 4), 0.2f);
  ASSERT(df.local_get_bool(2, 4));
  ASSERT(df.local_get_string(3, 4)->equals(q));

  delete a;
  delete b;
  delete c;
  delete d;
  delete e;
  delete ic;
  delete fc;
  delete bc;
  delete sc;
}

TEST_F(DataFrameTest, AddClone) {
  // Create a schema of Int Int
  Schema s("II");

  // Intiialize a DataFrame with the Schema above
  DataFrame df(s);

  // Create a row with the schema
  Row r(df.get_schema());

  // A String for the name of a new column
  String *a = new String("hello");

  // A String for the name of a row
  String *b = new String("my row");

  // Create 5 rows and add them to DataFrame
  for (size_t i = 0; i < 5; i++) {
    r.set(0, (int)i);
    r.set(1, (int)i + 1);
    df.add_row(r);
  }

  // Push back 5 floats to FloatColumn.
  FloatColumn *fc = new FloatColumn();
  for (size_t i = 0; i < 5; i++) {
    fc->push_back(0.1f);
  }

  // Clone the FloatColumn
  FloatColumn *fc_clone = fc->clone();
  delete fc;
  ASSERT(fc_clone != nullptr);

  // Add the clone to the DataFrame
  df.add_column(fc_clone);

  ASSERT_EQ(df.get_schema().col_type(0), 'I');
  ASSERT_EQ(df.get_schema().col_type(1), 'I');
  ASSERT_EQ(df.get_schema().col_type(2), 'F');

  delete a;
  delete b;
}

TEST_F(DataFrameTest, SchemaLength) {
  // A Schema of 'BBB'
  Schema s("BBB");

  // String names for Rows
  String *a = new String("hello");
  String *b = new String("world");

  // Add two rows to Schema
  s.add_row();
  s.add_row();

  // Create a new DataFrame with Schema 'BBB'
  DataFrame df(s);

  // Add three rows to DataFrame
  Row r(df.get_schema());
  for (size_t i = 0; i < 3; i++) {
    r.set(0, false);
    r.set(1, true);
    r.set(2, false);
    df.add_row(r);
  }

  // Check that the schema added to the DataFrame is
  // of length 3 because DataFrame needs to initialize
  // with all columns created empty.
  ASSERT_EQ(df.get_schema().length(), 3);

  delete a;
  delete b;
}

TEST_F(DataFrameTest, AddEmptyRow) {
  Schema s;
  DataFrame df(s); // Create a dataframe with an empty schema.
  Row r(s);        // Also create a row with that empty schema.

  ASSERT(df.nrows() == 0);
  df.add_row(r);           // Add it successfully.
  ASSERT(df.nrows() == 1); // We should now have exactly 1 row.

  df.fill_row(0, r); // We can fill our row with an empty row.
}

TEST_F(DataFrameTest, GetSchema2) {
  // A Schema of F
  Schema *s = new Schema("F");

  // Another Schema of FB
  Schema *s2 = new Schema("FB");

  // Create DataFrame with Schema: 'F'
  DataFrame df(*s);

  // A String for the name of a new Column
  String *a = new String("test");

  // A Bool Column of values
  BoolColumn *bc = new BoolColumn(4, true, false, false, true);

  // Add the new column
  df.add_column(bc);

  // Check that the new Schema is equal to 'FB'
  ASSERT(df.get_schema().equals(s2));

  delete s;
  delete s2;
  delete a;
  delete bc;
}

TEST_F(DataFrameTest, GetLastElement) {
  Schema s("S");
  DataFrame df(s);

  Row r(s);
  for (size_t i = 0; i < 10; i++) {
    r.set(0, new String("Foobar"));
    df.add_row(r);
  }

  String fb("Foobar");
  ASSERT(fb.equals(df.local_get_string(0, 9)));
}

TEST_F(DataFrameTest, TestClone) {
  Schema s("IS");
  DataFrame df(s);

  String *q = new String("Foobar");

  Row r(s);
  for (size_t i = 0; i < 10; i++) {
    r.set(0, (int)i);
    r.set(1, q->clone());
    df.add_row(r);
  }

  DataFrame *df2 = df.clone();
  for (size_t i = 0; i < 10; i++) {
    df2->fill_row(i, r);
    ASSERT_EQ(r.get_int(0), (int)i);
    ASSERT(r.get_string(1)->equals(q));
  }

  delete q;
  delete df2;
}

TEST_F(DataFrameTest, TestEquality) {
  Schema s("IS");
  DataFrame df(s);

  Row r(s);
  for (size_t i = 0; i < 10; i++) {
    r.set(0, (int)i);
    r.set(1, new String("Foobar"));
    df.add_row(r);
  }

  DataFrame *df2 = df.clone();

  ASSERT(df.equals(df2));

  DataFrame *df3 = new DataFrame(s);
  ASSERT(!df.equals(df3));

  delete df2;
  delete df3;
}
