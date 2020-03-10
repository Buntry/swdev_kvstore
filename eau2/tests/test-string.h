// lang: CwC
#pragma once

#include "../src/utils/object.h"
#include "../src/utils/string.h"
#include "test-macros.h"
#include <gtest/gtest.h>

/**
 * @brief Here's an example of a few interesting kinds of String to be used in
 * tests. s1 and s2 will contain equal Strings, while s3 will be a completely
 * new String. s4 is the concatenation of s3 and s1.
 *
 * @author griep.p@husky.neu.edu, colabella.a@husky.neu.edu
 */
class StringTest : public ::testing::Test {
public:
  String *s1;
  String *s2;
  String *s3;
  String *s4;

  void SetUp() {
    s1 = new String("test");
    s2 = new String("test");
    s3 = new String("hello");
    s4 = new String("hellotest");
  }

  void TearDown() {
    delete s1;
    delete s2;
    delete s3;
    delete s4;
  }
};

TEST_F(StringTest, HashInitializedToZero) { ASSERT_EQ(s1->hash_, 0); }
TEST_F(StringTest, HashIsNotZero) { ASSERT_NE(s1->hash(), 0); }
TEST_F(StringTest, HashConsistent) { ASSERT_EQ(s1->hash(), s1->hash()); }
TEST_F(StringTest, StringEqualsSameString) { ASSERT(s1->equals(s2)); }
TEST_F(StringTest, StringDoesNotEqualOtherString) { ASSERT(!s3->equals(s1)); }
TEST_F(StringTest, CharAtIsCorrect) { ASSERT_EQ(s1->at(1), 'e'); }
TEST_F(StringTest, StringSizeIsCorrect) { ASSERT_EQ(s1->size(), 4); }
TEST_F(StringTest, SameStringSameSize) { ASSERT_EQ(s1->size(), s2->size()); }

/** Comparison tests are removed since String has no compare method **/
// TEST_F(StringTest, CompareEqualIsZero) { ASSERT_EQ(s1->compare(s2), 0); }
// TEST_F(StringTest, CompareGreaterThan) { ASSERT_GT(s2->compare(s3), 0); }
// TEST_F(StringTest, CompareLessThan) { ASSERT_LT(s3->compare(s2), 0); }
// TEST_F(StringTest, CompareNotEqual) { ASSERT(s1->compare(s3) > 0); }
