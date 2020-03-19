// lang: CwC
#pragma once

#include "../src/utils/serializer.h"
#include "test-macros.h"
#include <gtest/gtest.h>

/**
 *
 */
class SerializerTest : public ::testing::Test {
public:
  Serializer ser;
};

TEST_F(SerializerTest, SizeT) {
  size_t x = 42;
  ser.write(x);

  Deserializer dser(ser.data());
  size_t y = dser.read_size_t();
  ASSERT_EQ(x, y);
}

TEST_F(SerializerTest, Int) {
  int x = 10;
  ser.write(x);

  Deserializer dser(ser.data());
}

TEST_F(SerializerTest, String) {
  String *a = new String("Alice");
  a->serialize(ser);

  Deserializer dser(ser.data());
  String *b = String::deserialize(dser);

  ASSERT(a->equals(b));
}
