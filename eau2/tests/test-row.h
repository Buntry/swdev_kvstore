#include "../src/dataframe.h"
#include "test-macros.h"
#include <gtest/gtest.h>

/**
 * @brief Here's are our unit tests for rows.
 *
 * @author griep.p@husky.neu.edu, colabella.a@husky.neu.edu
 */
class RowTest : public ::testing::Test {
public:
  /** Schema **/
  Schema *schema1;
  Schema *schema2;
  Schema *schema3;

  /** Rows **/
  Row *r1;
  Row *r2;
  Row *r3;

  /** Random Strings **/
  String *a;
  String *b;
  String *c;

  /** Initializes everything **/
  void SetUp() {
    a = new String("Alice");
    b = new String("Bob");
    c = new String("Charlie");
    schema1 = new Schema("IBSF");
    schema2 = new Schema("I");
    schema3 = new Schema();
    r1 = new Row(*schema1);
    r2 = new Row(*schema2);
    r3 = new Row(*schema3);
  }

  /** Tears everything down **/
  void TearDown() {
    delete a;
    delete b;
    delete c;
    delete r1;
    delete r2;
    delete r3;
    delete schema1;
    delete schema2;
    delete schema3;
  }
};

TEST_F(RowTest, Width) {
  ASSERT_EQ(r1->width(), 4);
  ASSERT_EQ(r2->width(), 1);
  ASSERT_EQ(r3->width(), 0);
}

TEST_F(RowTest, ColType) {
  ASSERT_EQ(r1->col_type(0), 'I');
  ASSERT_EQ(r1->col_type(1), 'B');
  ASSERT_EQ(r1->col_type(2), 'S');
  ASSERT_EQ(r1->col_type(3), 'F');
  ASSERT_EQ(r2->col_type(0), 'I');
}

TEST_F(RowTest, SetIdxGetIdx) {
  r1->set_idx(11);
  ASSERT_EQ(r1->get_idx(), 11);
}

TEST_F(RowTest, SetColGetCol) {
  r1->set(0, 1);
  r1->set(1, false);
  r1->set(2, a->clone());
  r1->set(3, 0.12f);

  ASSERT_EQ(r1->get_int(0), 1);
  ASSERT_EQ(r1->get_bool(1), false);
  ASSERT(r1->get_string(2)->equals(a));
  ASSERT_EQ(r1->get_float(3), 0.12f);
  ASSERT_FAIL(r1->get_bool(0));
  ASSERT_FAIL(r1->get_string(1));
  ASSERT_FAIL(r1->get_float(2));
  ASSERT_FAIL(r1->get_int(3));
}

TEST_F(RowTest, Reset) {
  String *s = new String("Beep");
  String *q = new String("Bops");

  Schema sc("S");
  Row r(sc);
  r.set(0, s->clone());

  assert(r.get_string(0)->equals(s));
  r.set(0, q->clone());
  assert(r.get_string(0)->equals(q));

  delete s;
  delete q;
}
