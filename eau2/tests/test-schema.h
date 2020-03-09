#include "../src/dataframe.h"
#include "test-macros.h"
#include <gtest/gtest.h>

/**
 * @brief Here's are our unit tests for schemas.
 *
 * @author griep.p@husky.neu.edu, colabella.a@husky.neu.edu
 */
class SchemaTest : public ::testing::Test {
public:
  /** Schema **/
  Schema *schema1;
  Schema *schema2;
  Schema *schema3;
  Schema *schema4;

  /** String **/
  String *a;
  String *b;

  /** Initializes everything **/
  void SetUp() {
    schema1 = new Schema("IBSF");
    schema2 = new Schema("I");
    schema3 = new Schema();
    schema4 = new Schema("IBSF");
    a = new String("test");
    b = new String("hello");
  }

  /** Tears everything down **/
  void TearDown() {
    delete schema1;
    delete schema2;
    delete schema3;
    delete schema4;
    delete a;
    delete b;
  }
};

TEST_F(SchemaTest, InitialWidthAndHeight) {
  ASSERT_EQ(schema1->width(), 4);
  ASSERT_EQ(schema2->width(), 1);
  ASSERT_EQ(schema3->width(), 0);
  ASSERT_EQ(schema1->length(), 0);
  ASSERT_EQ(schema2->length(), 0);
  ASSERT_EQ(schema3->length(), 0);
}

TEST_F(SchemaTest, Equals) {
  ASSERT(!schema1->equals(schema2));
  ASSERT(schema1->equals(schema4));
}

TEST_F(SchemaTest, AddColumn) {
  schema3->add_column('I', a);

  ASSERT_EQ(schema3->col_idx(a->c_str()), 0);
  ASSERT(schema3->col_name(0)->equals(a));
  ASSERT_EQ(schema3->col_type(0), 'I');
}

TEST_F(SchemaTest, AddRow) {
  schema4->add_row(a);

  ASSERT_EQ(schema4->row_idx(a->c_str()), 0);
  ASSERT(schema4->row_name(0)->equals(a));
}

TEST_F(SchemaTest, ColType) {
  ASSERT_EQ(schema1->col_type(0), 'I');
  ASSERT_EQ(schema1->col_type(1), 'B');
  ASSERT_EQ(schema1->col_type(2), 'S');
  ASSERT_EQ(schema1->col_type(3), 'F');
  // Undefined Behavior
  // ASSERT_EQ(schema3->col_type(0), ' ');
}

TEST_F(SchemaTest, PurgeRows) {
  schema2->add_row(a);
  schema2->add_row(b);
  schema2->purge_rows();

  ASSERT_EQ(schema2->length(), 0);
}
