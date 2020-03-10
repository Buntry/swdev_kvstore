// lang: CwC
#pragma once

#include "../src/array.h"
#include "../src/util.h"
#include "test-macros.h"
#include <gtest/gtest.h>

/**
 * @brief Testing the utility class with static methods.
 *
 * @author griep.p@husky.neu.edu, colabella.a@husky.neu.edu
 */
class UtilTest : public ::testing::Test {
public:
  Object *o1;
  Object *o2;
  Object *o3;
  String *s1;
  String *s2;
  String *s3;

  void SetUp() {
    o1 = new Object();
    o2 = new Object();
    o3 = o1;

    s1 = new String("foobar");
    s2 = new String("FOOBAR");
    s3 = new String("foobar");
  }

  void TearDown() {
    delete o1;
    delete o2;
    delete s1;
    delete s2;
    delete s3;
  }
};

TEST_F(UtilTest, BooleanPrimitiveEquality) {
  ASSERT(Util::equals(true, true));
  ASSERT(Util::equals(false, false));
}

TEST_F(UtilTest, IntegerPrimitiveEquality) {
  ASSERT(Util::equals(1, 1));
  ASSERT(!Util::equals(2, -19));
}

TEST_F(UtilTest, FloatPrimitiveEquality) {
  ASSERT(Util::equals(1.02f, 1.02f));
  ASSERT(Util::equals(2.000003f, 2.000003f));
  ASSERT(!Util::equals(2.03f, -2.03f));
}

TEST_F(UtilTest, DoublePrimitiveEquality) {
  // Double
  ASSERT(Util::equals((double)1.08, (double)1.08));
  ASSERT(!Util::equals((double)1.08, (double)1.06));
}

TEST_F(UtilTest, ObjectEquality) {
  ASSERT(Util::equals(o1, o1));
  ASSERT(!Util::equals(o1, o2));
  ASSERT(Util::equals(o1, o3));
}

TEST_F(UtilTest, StringEquality) {
  ASSERT(Util::equals(s1, s1));
  ASSERT(!Util::equals(s1, s2));
  ASSERT(Util::equals(s1, s3));
}

TEST_F(UtilTest, BooleanPrimitiveHashing) {
  ASSERT(Util::equals(true, true));
  ASSERT_EQ(Util::hash(true), Util::hash(true));
  ASSERT(Util::hash(true) != Util::hash(false));
}

TEST_F(UtilTest, IntegerPrimitiveHashing) {
  ASSERT_EQ(Util::hash(1), Util::hash(1));
  ASSERT(Util::hash(1) != Util::hash(2));
  ASSERT(Util::hash(1) != Util::hash(-1));
}

TEST_F(UtilTest, NullptrCloning) { ASSERT(Util::clone(nullptr) == nullptr); }

TEST_F(UtilTest, ObjectCloning) { ASSERT(Util::clone(o1) == o1->clone()); }
TEST_F(UtilTest, StringCloning) {
  String *s1_clone = dynamic_cast<String *>(Util::clone(s1));
  ASSERT(s1_clone != nullptr);
  ASSERT(s1_clone->equals(s1));
}

TEST_F(UtilTest, SemiAdvancedCloning) {
  IntArray ia;
  ia.push_back(1);
  ia.push_back(2);
  ia.push_back(3);

  IntArray *ia_cloned = ia.clone();
  ASSERT(ia_cloned != nullptr);
  ASSERT(ia.equals(ia_cloned));
}

TEST_F(UtilTest, AdvancedCloning) {
  StringArray sa;
  sa.push_back(new String("Apples"));
  sa.push_back(new String("Bananas"));
  sa.push_back(new String("Cereal"));

  StringArray *sa_cloned = sa.clone();
  ASSERT(sa_cloned != nullptr);
  ASSERT(sa.equals(sa_cloned));
}