// lang: CwC
#pragma once

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

TEST_F(UtilTest, PrimitiveEquality) {
  // Booleans
  ASSERT(Util::equals(true, true));
  ASSERT(Util::equals(false, false));

  // Integers
  ASSERT(Util::equals(1, 1));
  ASSERT(!Util::equals(2, -19));

  // Float
  ASSERT(Util::equals(1.02f, 1.02f));
  ASSERT(Util::equals(2.000003f, 2.000003f));
  ASSERT(!Util::equals(2.03f, -2.03f));

  // Double
  ASSERT(Util::equals((double)1.08, (double)1.08));
  ASSERT(!Util::equals((double)1.08, (double)1.06));
}

TEST_F(UtilTest, ObjectEquality) {
  ASSERT(Util::equals(o1, o1));
  ASSERT(!Util::equals(o1, o2));
  ASSERT(Util::equals(o1, o3));

  ASSERT(Util::equals(s1, s1));
  ASSERT(!Util::equals(s1, s2));
  ASSERT(Util::equals(s1, s3));
}

TEST_F(UtilTest, PrimitiveHashing) {
  // Booleans
  ASSERT(Util::equals(true, true));
  ASSERT_EQ(Util::hash(true), Util::hash(true));
  ASSERT(Util::hash(true) != Util::hash(false));
}