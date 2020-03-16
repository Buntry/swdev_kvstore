// lang: CwC
#pragma once

#include "../src/utils/map.h"
#include "test-macros.h"
#include <gtest/gtest.h>

/**
 * @brief Map Unit Tests
 * @author griep.p@husky.neu.edu, colabella.a@husky.neu.edu
 */
class MapTest : public ::testing::Test {
public:
  Map m;
};

TEST_F(MapTest, OnePut) {
  String key("Key");
  String value("Value");

  ASSERT_EQ(m.size(), 0);
  ASSERT(!m.contains_key(&key));
  m.put(&key, &value);

  ASSERT(m.contains_key(&key));
  ASSERT(value.equals(m.get(&key)));
}

TEST_F(MapTest, MultiplePuts) {
  String k("key");
  String v1("Alice");
  String v2("Bob");

  ASSERT_EQ(m.size(), 0);
  ASSERT(!m.contains_key(&k));
  m.put(&k, &v1);

  ASSERT_EQ(m.size(), 1);
  ASSERT(v1.equals(m.get(&k)));
  ASSERT(!v2.equals(m.get(&k)));
  m.put(&k, &v2);

  ASSERT_EQ(m.size(), 1);
  ASSERT(!v1.equals(m.get(&k)));
  ASSERT(v2.equals(m.get(&k)));
}

/** Helper function to generate a random string. **/
void gen_random(char *s, const int len) {
  static const char alphanum[] = "0123456789"
                                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz";

  for (int i = 0; i < len; ++i) {
    s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  s[len] = 0;
}

TEST_F(MapTest, ExtensivePuts) {
  ASSERT_EQ(m.size(), 0);

  int limit = 100 * 1000;
  for (int i = 1; i < limit; i++) {
    char buf[26] = {0};
    gen_random(buf, 25);

    String s(buf);
    m.put(&s, &s);
    ASSERT_EQ(m.size(), i);
  }

  Array *keys = m.keys();
  for (size_t i = 0; i < keys->size(); i++) {
    ASSERT(m.contains_key(keys->get(i)));
    m.remove(keys->get(i));
  }

  ASSERT_EQ(m.size(), 0);
}
