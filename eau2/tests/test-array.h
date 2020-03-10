// lang: CwC
#pragma once

#include "../src/array.h"
#include "../src/object.h"
#include "test-macros.h"
#include <gtest/gtest.h>

/**
 * @brief Here's are our unit tests for arrays.
 *
 * @author griep.p@husky.neu.edu, colabella.a@husky.neu.edu
 */
class ArrayTest : public ::testing::Test {
public:
  Array *a;
  BoolArray *ba;
  IntArray *ia;
  FloatArray *fa;
  StringArray *sa;

  String *b;
  String *c;
  String *d;

  void SetUp() {
    a = new Array();
    ba = new BoolArray();
    ia = new IntArray();
    fa = new FloatArray();
    sa = new StringArray();

    b = new String("bob");
    c = new String("charlie");
    d = new String("diane");
  }

  void TearDown() {
    delete a;
    delete ba;
    delete ia;
    delete fa;
    delete sa;

    delete b;
    delete c;
    delete d;
  }
};

// SIMPLE PUSH BACK -----------------------------------------
TEST_F(ArrayTest, SimplePushBack) {
  a->push_back(b);
  ASSERT(a->get(0)->equals(b));
  ASSERT_EQ(a->size(), 1);
}

TEST_F(ArrayTest, StringArraySimplePushBack) {
  sa->push_back(b);
  ASSERT(sa->get(0)->equals(b));
  ASSERT_EQ(sa->size(), 1);
}

TEST_F(ArrayTest, FloatArraySimplePushBack) {
  fa->push_back(2.01f);
  ASSERT(fa->get(0) == 2.01f);
  ASSERT_EQ(fa->size(), 1);
}

TEST_F(ArrayTest, IntArraySimplePushBack) {
  ia->push_back(-222);
  ASSERT_EQ(ia->get(0), -222);
  ASSERT_EQ(ia->size(), 1);
}

TEST_F(ArrayTest, BoolArraySimplePushBack) {
  ba->push_back(false);
  ASSERT(!ba->get(0));
  ASSERT_EQ(ba->size(), 1);
}

// MULTIPLE ADDS -----------------------------------------
TEST_F(ArrayTest, MultipleAdds) {
  a->push_back(b);
  a->push_back(c);
  a->add(1, d);

  ASSERT(a->get(0)->equals(b));
  ASSERT(a->get(1)->equals(d));
  ASSERT(a->get(2)->equals(c));

  Array *other_a = new Array();
  other_a->push_back(b);
  other_a->push_back(d);
  other_a->push_back(c);

  ASSERT(a->equals(other_a));
  ASSERT_EQ(a->size(), 3);
  ASSERT_EQ(a->size(), other_a->size());
}

TEST_F(ArrayTest, StringArrayMultipleAdds) {
  sa->push_back(b);
  sa->push_back(c);
  sa->add(1, d);

  ASSERT(sa->get(0)->equals(b));
  ASSERT(sa->get(1)->equals(d));
  ASSERT(sa->get(2)->equals(c));

  StringArray *other_sa = new StringArray();
  other_sa->push_back(b);
  other_sa->push_back(d);
  other_sa->push_back(c);

  ASSERT(sa->equals(other_sa));
  ASSERT_EQ(sa->size(), 3);
  ASSERT_EQ(sa->size(), other_sa->size());
}

TEST_F(ArrayTest, FloatArrayMultipleAdds) {
  fa->push_back(1.02f);
  fa->push_back(2.02f);
  fa->add(1, 3.02f);

  ASSERT_EQ(fa->get(0), 1.02f);
  ASSERT_EQ(fa->get(1), 3.02f);
  ASSERT_EQ(fa->get(2), 2.02f);

  FloatArray *other_fa = new FloatArray();
  other_fa->push_back(1.02f);
  other_fa->push_back(3.02f);
  other_fa->push_back(2.02f);

  ASSERT(fa->equals(other_fa));
  ASSERT_EQ(fa->size(), 3);
  ASSERT_EQ(fa->size(), other_fa->size());
}

TEST_F(ArrayTest, IntArrayMultipleAdds) {
  ia->push_back(1);
  ia->push_back(2);
  ia->add(1, 3);

  ASSERT_EQ(ia->get(0), 1);
  ASSERT_EQ(ia->get(1), 3);
  ASSERT_EQ(ia->get(2), 2);

  IntArray *other_ia = new IntArray();
  other_ia->push_back(1);
  other_ia->push_back(3);
  other_ia->push_back(2);

  ASSERT(ia->equals(other_ia));
  ASSERT_EQ(ia->size(), 3);
  ASSERT_EQ(ia->size(), other_ia->size());
}

TEST_F(ArrayTest, BoolArrayMultipleAdds) {
  ba->push_back(false);
  ba->push_back(true);
  ba->add(1, false);

  ASSERT_EQ(ba->get(0), false);
  ASSERT_EQ(ba->get(1), false);
  ASSERT_EQ(ba->get(2), true);

  BoolArray *other_ba = new BoolArray();
  other_ba->push_back(false);
  other_ba->push_back(false);
  other_ba->push_back(true);

  ASSERT(ba->equals(other_ba));
  ASSERT_EQ(ba->size(), 3);
  ASSERT_EQ(ba->size(), other_ba->size());
}

// INVOKE RESIZE -----------------------------------------
TEST_F(ArrayTest, InvokeResize) {
  for (size_t i = 0; i < CHUNK_SIZE * 2; i++) {
    a->push_back(new Object());
  }

  while (a->size() != 0) {
    delete a->remove(a->size() - 1);
  }
}

TEST_F(ArrayTest, StringArrayInvokeResize) {
  for (size_t i = 0; i < CHUNK_SIZE * 2; i++) {
    sa->push_back(new String("our code is fubar"));
  }

  while (sa->size() != 0) {
    delete sa->remove(sa->size() - 1);
  }
}

TEST_F(ArrayTest, FloatArrayInvokeResize) {
  for (size_t i = 0; i < CHUNK_SIZE * 2; i++) {
    fa->push_back((i % 10) + 0.002342);
  }
}

TEST_F(ArrayTest, IntArrayInvokeResize) {
  for (size_t i = 0; i < CHUNK_SIZE * 2; i++) {
    ia->push_back((i % 23));
  }
}

TEST_F(ArrayTest, BoolArrayInvokeResize) {
  for (size_t i = 0; i < CHUNK_SIZE * 2; i++) {
    ba->push_back((i % 2) == 0);
  }
}

// CLEARS -------------------------------------------------
TEST_F(ArrayTest, Clears) {
  a->push_back(b);
  a->add(0, sa);
  a->push_back(ba);
  ASSERT_EQ(a->size(), 3);
  a->clear();
  ASSERT_EQ(a->size(), 0);
  a->push_back(a);
  ASSERT_EQ(a->size(), 1);
}

TEST_F(ArrayTest, StringArrayClears) {
  sa->push_back(b);
  sa->add(0, c);
  sa->push_back(d);
  ASSERT_EQ(sa->size(), 3);
  sa->clear();
  ASSERT_EQ(sa->size(), 0);
  sa->push_back(c);
  ASSERT_EQ(sa->size(), 1);
}

TEST_F(ArrayTest, FloatArrayClears) {
  fa->push_back(1.0f);
  fa->add(0, -2.000001f);
  fa->push_back(3.09f);
  ASSERT_EQ(fa->size(), 3);
  fa->clear();
  ASSERT_EQ(fa->size(), 0);
  fa->push_back(3.01f);
  ASSERT_EQ(fa->size(), 1);
}

TEST_F(ArrayTest, IntArrayClears) {
  ia->push_back(1);
  ia->add(0, -2);
  ia->push_back(3);
  ASSERT_EQ(ia->size(), 3);
  ia->clear();
  ASSERT_EQ(ia->size(), 0);
  ia->push_back(3);
  ASSERT_EQ(ia->size(), 1);
}

TEST_F(ArrayTest, BoolArrayClears) {
  ba->push_back(true);
  ba->add(0, false);
  ba->push_back(false);
  ASSERT_EQ(ba->size(), 3);
  ba->clear();
  ASSERT_EQ(ba->size(), 0);
  ba->push_back(false);
  ASSERT_EQ(ba->size(), 1);
}

// HANDLES NULLPTR -----------------------------------
TEST_F(ArrayTest, HandlesNullptr) {
  ASSERT_EQ(a->size(), 0);
  a->push_back(nullptr);
  ASSERT_EQ(a->size(), 1);
  ASSERT(a->get(0) == nullptr);
  a->add(0, nullptr);
  ASSERT(a->get(1) == nullptr);

  Array *other_a = new Array();
  other_a->add_all(0, a);
  ASSERT(a->equals(other_a));
}

TEST_F(ArrayTest, StringArrayHandlesNullptr) {
  ASSERT_EQ(sa->size(), 0);
  sa->push_back(nullptr);
  ASSERT_EQ(sa->size(), 1);
  ASSERT(sa->get(0) == nullptr);
  sa->add(0, b);
  ASSERT(sa->get(1) == nullptr);

  Array *other_a = new Array();
  other_a->add_all(0, a);
  ASSERT(a->equals(other_a));
}

// ---- Cloning ----
TEST_F(ArrayTest, StringArrayCanClone) {
  sa->push_back(b);
  sa->push_back(c);
  sa->push_back(d);

  StringArray *sa2 = sa->clone();
  ASSERT(sa->equals(sa2));

  delete sa2;
}