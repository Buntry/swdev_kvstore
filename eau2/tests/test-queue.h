// lang: CwC
#pragma once

#include <gtest/gtest.h>

#include "../src/utils/queue.h"
#include "test-macros.h"

/**
 * @brief Here's are the unit tests for the Queue class.
 */
class QueueTest : public ::testing::Test {
 public:
  Queue q;
  StringQueue sq;

  String *a, *b, *c;
  void SetUp() {
    a = new String("Alice");
    b = new String("Bob");
    c = new String("Charlie");
  }

  void TearDown() {
    delete a;
    delete b;
    delete c;
  }
};

TEST_F(QueueTest, Size) {
  ASSERT_EQ(q.len(), 0);
  q.push(a);
  ASSERT_EQ(q.len(), 1);
  ASSERT(a->equals(q.pop()));
}

TEST_F(QueueTest, InvokeResize) {
  size_t limit = 100 * 1000;
  for (size_t i = 0; i < limit; i++) {
    sq.push(a);
    sq.push(b);
    sq.pop();
    ASSERT_EQ(sq.len(), i + 1);
  }

  while (sq.len() != 0) {
    sq.pop();
  }

  // Can push after emptying
  sq.push(c);
  ASSERT_EQ(sq.len(), 1);
}
