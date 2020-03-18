#pragma once

#include "../src/utils/pack.h"
#include "test-macros.h"
#include <gtest/gtest.h>

/**
 * @brief Unit tests for Packing/Unpacking objects.
 *
 * @author griep.p@husky.neu.edu, colabella.a@husky.neu.edu
 */
class PackTest : public ::testing::Test {
public:
  char *buf;
  char *buf2;
  size_t max_size = 1024;

  void SetUp() {
    buf = new char[max_size];
    buf2 = buf;
  }

  void TearDown() {}
};

TEST_F(PackTest, Doubles) {
  packd(buf, 0.1f);
  packd(buf, 0.2f);
  packd(buf, 0.3f);

  ASSERT_EQ(0.1f, unpackd(buf2));
  ASSERT_EQ(0.2f, unpackd(buf2));
  ASSERT_EQ(0.3f, unpackd(buf2));
}

TEST_F(PackTest, Ints) {
  packi(buf, 1);
  packi(buf, 2);
  packi(buf, 3);

  ASSERT_EQ(1, unpacki(buf2));
  ASSERT_EQ(2, unpacki(buf2));
  ASSERT_EQ(3, unpacki(buf2));
}

TEST_F(PackTest, SizeT) {
  packst(buf, 1);
  packst(buf, 2);
  packst(buf, 3);

  ASSERT_EQ(1, unpackst(buf2));
  ASSERT_EQ(2, unpackst(buf2));
  ASSERT_EQ(3, unpackst(buf2));
}

TEST_F(PackTest, String) {
  String *s1 = new String("one");
  String *s2 = new String("two");
  String *s3 = new String("three");
  String *s1_clone = new String("one");
  String *s2_clone = new String("two");
  String *s3_clone = new String("three");

  packs(buf, s1);
  packs(buf, s2);
  packs(buf, s3);

  ASSERT(unpacks(buf2)->equals(s1_clone));
  ASSERT(unpacks(buf2)->equals(s2_clone));
  ASSERT(unpacks(buf2)->equals(s3_clone));
}
