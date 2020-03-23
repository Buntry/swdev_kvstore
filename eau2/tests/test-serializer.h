// lang: CwC
#pragma once

#include "../src/utils/serializer.h"
#include "test-macros.h"
#include <gtest/gtest.h>

/**
 * Serializer and Deserializer Unit Tests
 */
class SerializerTest : public ::testing::Test {
public:
  Serializer ser;
};

TEST_F(SerializerTest, SizeT) {
  size_t x1 = 0;
  size_t x2 = 10;
  ser.write(x1);
  ser.write(x2);

  Deserializer dser(*ser.data());
  size_t y1 = dser.read_size_t();
  size_t y2 = dser.read_size_t();

  ASSERT_EQ(x1, y1);
  ASSERT_EQ(x2, y2);
}

TEST_F(SerializerTest, Int) {
  int x1 = -10;
  int x2 = 10;
  ser.write(x1);
  ser.write(x2);

  Deserializer dser(*ser.data());
  int y1 = dser.read_int();
  int y2 = dser.read_int();

  ASSERT_EQ(x1, y1);
  ASSERT_EQ(x2, y2);
}

TEST_F(SerializerTest, Char) {
  char x1 = '\0';
  char x2 = 'z';
  ser.write(x1);
  ser.write(x2);

  Deserializer dser(*ser.data());
  char y1 = dser.read_char();
  char y2 = dser.read_char();

  ASSERT_EQ(x1, y1);
  ASSERT_EQ(x2, y2);
}

TEST_F(SerializerTest, Chars) {
  char x1[] = {'a', 'b', 'c'};
  char x2[] = {'x', '\0', 'z'};
  ser.write(x1, 3);
  ser.write(x2, 3);

  Deserializer dser(*ser.data());
  char *y1 = dser.read_chars(3);
  char *y2 = dser.read_chars(3);

  for (size_t i = 0; i < 3; i++) {
    ASSERT_EQ(x1[i], y1[i]);
    ASSERT_EQ(x2[i], y2[i]);
  }
}

TEST_F(SerializerTest, Bool) {
  bool x1 = true;
  bool x2 = false;
  ser.write(x1);
  ser.write(x2);

  Deserializer dser(*ser.data());
  bool y1 = dser.read_bool();
  bool y2 = dser.read_bool();

  ASSERT_EQ(x1, y1);
  ASSERT_EQ(x2, y2);
}

TEST_F(SerializerTest, Float) {
  float x1 = 10.03f;
  float x2 = -12.002f;
  ser.write(x1);
  ser.write(x2);

  Deserializer dser(*ser.data());
  float y1 = dser.read_float();
  float y2 = dser.read_float();

  ASSERT_EQ(x1, y1);
  ASSERT_EQ(x2, y2);
}

TEST_F(SerializerTest, Double) {
  double x1 = 10.03;
  double x2 = -12.002;
  ser.write(x1);
  ser.write(x2);

  Deserializer dser(*ser.data());
  double y1 = dser.read_double();
  double y2 = dser.read_double();

  ASSERT_DOUBLE_EQ(x1, y1);
  ASSERT_DOUBLE_EQ(x2, y2);
}

TEST_F(SerializerTest, String) {
  String *s = new String("Apple juice");

  Serializer ser;
  s->serialize(ser);

  Deserializer dser(*ser.data());
  String *q = String::deserialize(dser);

  assert(s->equals(q));

  delete s;
  delete q;
}

TEST_F(SerializerTest, InformationalMethods) {
  String *s = new String("Software development is fun!");

  ASSERT_EQ(ser.length(), 0);
  ASSERT_GE(ser.num_chunks(), 0);

  s->serialize(ser);
  ASSERT_GT(ser.length(), 0);
  ASSERT_GT(ser.num_chunks(), 0);

  delete s;
}

TEST_F(SerializerTest, FailOnBadCursor) {
  Deserializer dser(new CharArray());
  ASSERT_FAIL(dser.read_char());
}

TEST_F(SerializerTest, TestBoolColumn) {
  BoolColumn bc;
  bc.push_back(true);
  bc.push_back(false);
  bc.push_back_missing();
  bc.push_back_missing();

  ser.write(&bc);
  Deserializer dser(*ser.data());

  BoolColumn *bc2 = Column::deserialize(dser)->as_bool();
  for (size_t i = 0; i < bc.size(); i++) {
    ASSERT_EQ(bc.is_missing(i), bc2->is_missing(i));
    if (!bc.is_missing(i)) {
      ASSERT_EQ(bc.get(i), bc2->get(i));
    }
  }
  delete bc2;
}

TEST_F(SerializerTest, TestIntColumn) {
  IntColumn ic;
  ic.push_back(-25);
  ic.push_back_missing();
  ic.push_back(200);
  ic.push_back_missing();

  ser.write(&ic);
  Deserializer dser(*ser.data());

  IntColumn *ic2 = Column::deserialize(dser)->as_int();
  for (size_t i = 0; i < ic.size(); i++) {
    ASSERT_EQ(ic.is_missing(i), ic2->is_missing(i));
    if (!ic.is_missing(i)) {
      ASSERT_EQ(ic.get(i), ic2->get(i));
    }
  }
  delete ic2;
}

TEST_F(SerializerTest, TestFloatColumn) {
  FloatColumn fc;
  fc.push_back(-25.003f);
  fc.push_back_missing();
  fc.push_back(200.999f);
  fc.push_back_missing();

  ser.write(&fc);
  Deserializer dser(*ser.data());

  FloatColumn *fc2 = Column::deserialize(dser)->as_float();
  for (size_t i = 0; i < fc.size(); i++) {
    ASSERT_EQ(fc.is_missing(i), fc2->is_missing(i));
    if (!fc.is_missing(i)) {
      ASSERT_FLOAT_EQ(fc.get(i), fc2->get(i));
    }
  }
  delete fc2;
}

TEST_F(SerializerTest, TestStringColumn) {
  String *a = new String("Alice");
  String *b = new String("Bob");
  String *c = new String("Charlie");

  StringColumn *sc = new StringColumn(3, a, b, nullptr, c);

  ser.write(sc);

  Deserializer dser(*ser.data());
  Column *qc = Column::deserialize(dser);

  assert(sc->size() == qc->size());
  for (size_t i = 0; i < sc->size(); i++) {
    assert(Util::equals(sc->get(i), qc->as_string()->get(i)));
  }

  delete a;
  delete b;
  delete c;

  delete sc;
  delete qc;
}

TEST_F(SerializerTest, TestSchema) {
  Schema s("BFISIFS");
  ser.write(&s);

  Deserializer dser(*ser.data());
  Schema *q = Schema::deserialize(dser);

  ASSERT(s.equals(q));
  delete q;
}

TEST_F(SerializerTest, TestDataFrame) {}
