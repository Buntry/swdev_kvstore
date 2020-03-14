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
  ASSERT_EQ(q.size(), 0);
  q.push(a);
  ASSERT_EQ(q.size(), 1);
  ASSERT(a->equals(q.pop()));
}

TEST_F(QueueTest, InvokeResize) {
  size_t limit = 100 * 1000;
  for (size_t i = 0; i < limit; i++) {
    sq.push(a);
    sq.push(b);
    sq.pop();
    ASSERT_EQ(sq.size(), i + 1);
  }

  while (sq.size() != 0) {
    sq.pop();
  }

  // Can push after emptying
  sq.push(c);
  ASSERT_EQ(sq.size(), 1);
}

TEST_F(QueueTest, PushPopGet) {
  for (size_t i = 0; i < CHUNK_SIZE * 4; i++) {
    sq.push(a);
    sq.push(b);
    String *s = sq.pop();
    if (i % 2 == 0) {
      ASSERT(a->equals(s));
    } else {
      ASSERT(b->equals(s));
    }
  }
}

TEST_F(QueueTest, ResizeInOneChunk) {
  for (size_t i = 0; i < CHUNK_SIZE / 2; i++) {
    sq.push(a);
    ASSERT(sq.peek() != nullptr);
    sq.pop();
  }

  for (size_t i = 0; i < CHUNK_SIZE * 2; i++) {
    sq.push(a);
    ASSERT(sq.peek() != nullptr);
    sq.push(a);
    ASSERT(sq.peek() != nullptr);
    String *s = sq.pop();

    ASSERT(a->equals(s));
  }
}
