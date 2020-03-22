#include "../src/store/dataframe.h"
#include "test-macros.h"
#include <gtest/gtest.h>

/**
 * @brief Here's are our unit tests for columns.
 *
 * @author griep.p@husky.neu.edu, colabella.a@husky.neu.edu
 */
class ColumnTest : public ::testing::Test {
public:
  /** Originally empty columns **/
  Column *nc;
  StringColumn *sc;
  FloatColumn *fc;
  IntColumn *ic;
  BoolColumn *bc;

  /** Non-empty columns **/
  StringColumn *nesc;
  FloatColumn *nefc;
  IntColumn *neic;
  BoolColumn *nebc;
  size_t initial_size;

  /** Random Strings **/
  String *a;
  String *b;
  String *c;

  /** Initializes everything **/
  void SetUp() {
    nc = new Column();
    sc = new StringColumn();
    fc = new FloatColumn();
    ic = new IntColumn();
    bc = new BoolColumn();

    a = new String("Alice");
    b = new String("Bob");
    c = new String("Charlie");

    initial_size = 3;
    nesc = new StringColumn(initial_size, a->clone(), b->clone(), c->clone());
    nefc = new FloatColumn(initial_size, 1.02f, 2.02f, 3.02f);
    neic = new IntColumn(initial_size, -1, -2, -3);
    nebc = new BoolColumn(initial_size, true, false, true);
  }

  /** Tears everything down **/
  void TearDown() {
    delete nc;
    delete sc;
    delete fc;
    delete ic;
    delete bc;
    delete nesc;
    delete nefc;
    delete neic;
    delete nebc;
    delete a;
    delete b;
    delete c;
  }

  /** Tests if two columns have equal elements. Note that
   * Columns, by default have pointer equality. **/
  bool equal_columns(Column *c1, Column *c2) {
    if (c1->size() != c2->size() || c1->get_type() != c2->get_type()) {
      return false;
    } else {
      switch (c1->get_type()) {
      case '\0':
        return c1 == c2;
      case 'I':
        return c1->as_int()->vals_.equals(&c2->as_int()->vals_);
      case 'F':
        return c1->as_float()->vals_.equals(&c2->as_float()->vals_);
      case 'S':
        return c1->as_string()->vals_.equals(&c2->as_string()->vals_);
      case 'B':
        return c1->as_bool()->vals_.equals(&c2->as_bool()->vals_);
      }
    }
    return false;
  }
};

TEST_F(ColumnTest, FixtureRuns) {}
TEST_F(ColumnTest, HasProperSize) {
  ASSERT_EQ(sc->size(), 0);
  ASSERT_EQ(fc->size(), 0);
  ASSERT_EQ(ic->size(), 0);
  ASSERT_EQ(bc->size(), 0);
  ASSERT_EQ(nesc->size(), initial_size);
  ASSERT_EQ(nefc->size(), initial_size);
  ASSERT_EQ(neic->size(), initial_size);
  ASSERT_EQ(nebc->size(), initial_size);
}

TEST_F(ColumnTest, AsType) {
  ASSERT(nc->as_bool() == nullptr);
  ASSERT(nc->as_int() == nullptr);
  ASSERT(nc->as_float() == nullptr);
  ASSERT(nc->as_string() == nullptr);
  ASSERT(sc->as_bool() == nullptr);
  ASSERT(sc->as_int() == nullptr);
  ASSERT(sc->as_float() == nullptr);
  ASSERT(sc->as_string() != nullptr);
  ASSERT(fc->as_bool() == nullptr);
  ASSERT(fc->as_int() == nullptr);
  ASSERT(fc->as_float() != nullptr);
  ASSERT(fc->as_string() == nullptr);
  ASSERT(ic->as_bool() == nullptr);
  ASSERT(ic->as_int() != nullptr);
  ASSERT(ic->as_float() == nullptr);
  ASSERT(ic->as_string() == nullptr);
  ASSERT(bc->as_bool() != nullptr);
  ASSERT(bc->as_int() == nullptr);
  ASSERT(bc->as_float() == nullptr);
  ASSERT(bc->as_string() == nullptr);
}

TEST_F(ColumnTest, DeathOnIncorrectPushBack) {
  ASSERT_FAIL(nc->push_back(a));
  ASSERT_FAIL(nc->push_back(1.05f));
  ASSERT_FAIL(nc->push_back(-222));
  ASSERT_FAIL(nc->push_back(true));
}

TEST_F(ColumnTest, GetsType) {
  ASSERT_EQ(nc->get_type(), '\0');
  ASSERT_EQ(sc->get_type(), 'S');
  ASSERT_EQ(fc->get_type(), 'F');
  ASSERT_EQ(ic->get_type(), 'I');
  ASSERT_EQ(bc->get_type(), 'B');
}

TEST_F(ColumnTest, Clones) {
  ASSERT(nc->clone() == nullptr);
  Column *sc2 = nesc->clone();
  ASSERT(equal_columns(sc2, nesc));
  delete sc2;
  Column *fc2 = nefc->clone();
  ASSERT(equal_columns(fc2, nefc));
  delete fc2;
  Column *ic2 = neic->clone();
  ASSERT(equal_columns(ic2, neic));
  delete ic2;
  Column *bc2 = nebc->clone();
  ASSERT(equal_columns(bc2, nebc));
  delete bc2;
}

TEST_F(ColumnTest, PushBack) {
  bc->push_back(true);
  fc->push_back(0.1f);
  ic->push_back(12);
  sc->push_back(a);

  ASSERT_EQ(bc->get(0), true);
  ASSERT_EQ(fc->get(0), 0.1f);
  ASSERT_EQ(ic->get(0), 12);
  ASSERT(sc->get(0)->equals(a));
  ASSERT_EQ(bc->size(), 1);
  ASSERT_EQ(fc->size(), 1);
  ASSERT_EQ(ic->size(), 1);
  ASSERT_EQ(sc->size(), 1);
}

TEST_F(ColumnTest, TestMissing) {
  ASSERT_EQ(nesc->size(), initial_size);
  nesc->push_back_missing();
  ASSERT_EQ(nesc->size(), initial_size + 1);

  String *a = new String("Garfield");
  nesc->push_back(a);
  ASSERT_EQ(nesc->size(), initial_size + 2);

  ASSERT(!nesc->is_missing(0));
  ASSERT(nesc->is_missing(initial_size));
  ASSERT(!nesc->is_missing(initial_size + 1));
  delete a;
}
