// lang: CwC
#pragma once

#include "../src/utils/object.h"
#include "test-macros.h"
#include <gtest/gtest.h>

/**
 * @brief Here's an example of a few interesting kinds of Objects to be used in
 * tests. o1 and o2 will point to the same object, while o3 will be a completely
 * new object.
 *
 * @author griep.p@husky.neu.edu, colabella.a@husky.neu.edu
 */
class ObjectTest : public ::testing::Test {
public:
  Object *o1;
  Object *o2;
  Object *o3;

  void SetUp() {
    o1 = new Object();
    o2 = o1;
    o3 = new Object();
  }

  void TearDown() {
    delete o1;
    delete o3;
  }
};

TEST_F(ObjectTest, HashInitializedToZero) { ASSERT_EQ(o1->hash_, 0); }
TEST_F(ObjectTest, HashIsNotZero) { ASSERT_NE(o1->hash(), 0); }
TEST_F(ObjectTest, HashConsistent) { ASSERT_EQ(o1->hash(), o1->hash()); }
TEST_F(ObjectTest, ObjectEqualsSelf) { ASSERT(o1->equals(o2)); }
TEST_F(ObjectTest, ObjectsInherentlyInequal) { ASSERT(!o3->equals(o1)); }
