// lang: CwC
#pragma once

#include "../src/dataframe.h"
#include "test-macros.h"
#include <gtest/gtest.h>

/** This is the test that shows that pmap is faster than regular map. **/
class DataFrameSpeedTest : public ::testing::Test {
public:
  DataFrame *df;
  int true_answer, xor_answer;

  void SetUp() {
    Schema s("I");
    df = new DataFrame(s);
    true_answer = 0;
    xor_answer = 0;

    Row r(df->get_schema());
    for (int i = 0; i < 10000000; i++) {
      true_answer += i;
      xor_answer ^= i;
      r.set(0, i);
      df->add_row(r);
    }
  }

  void TearDown() { delete df; }
};

/** This Fielder & Rower combo adds all the integers
 *  in a given DataFrame. It will be used for our
 *  speed test. **/
class AddAllFielder : public Fielder {
public:
  int localSum = 0;
  void start(size_t idx) { localSum = 0; }
  void accept(int v) { localSum += v; }
  int sum() { return localSum; }
};

class AddAllRower : public Rower {
public:
  AddAllFielder aaf;
  int localSum = 0;

  bool accept(Row &r) {
    r.visit(r.get_idx(), aaf);
    localSum += aaf.sum();
    return true;
  }

  int sum() { return localSum; }

  void join_delete(Rower *r) {
    AddAllRower *aar = dynamic_cast<AddAllRower *>(r);
    if (aar != nullptr) {
      localSum += aar->sum();
    }
    delete r;
  }

  Rower *clone() { return new AddAllRower(); }
};

/** This Fielder & Rower combo XORs all integers
 *  in a given DataFrame. It will be used for our
 *  speed test. **/
class XORAllFielder : public Fielder {
public:
  int local = 0;
  void start(size_t idx) { local = 0; }
  void accept(int v) { local ^= v; }
  int sum() { return local; }
};

class XORAllRower : public Rower {
public:
  XORAllFielder aaf;
  int local = 0;

  bool accept(Row &r) {
    r.visit(r.get_idx(), aaf);
    local ^= aaf.sum();
    return true;
  }

  int sum() { return local; }

  void join_delete(Rower *r) {
    XORAllRower *aar = dynamic_cast<XORAllRower *>(r);
    if (aar != nullptr) {
      local ^= aar->sum();
    }
    delete r;
  }

  Rower *clone() { return new XORAllRower(); }
};

TEST_F(DataFrameSpeedTest, MapAtTenMillion) {
  AddAllRower aar;
  df->map(aar);
  ASSERT_EQ(true_answer, aar.sum());
}

TEST_F(DataFrameSpeedTest, PMapAtTenMillion) {
  AddAllRower aar;
  df->pmap(aar);
  ASSERT_EQ(true_answer, aar.sum());
}

TEST_F(DataFrameSpeedTest, XORMapAtTenMillion) {
  XORAllRower aar;
  df->map(aar);
  ASSERT_EQ(xor_answer, aar.sum());
}

TEST_F(DataFrameSpeedTest, XORPMapAtTenMillion) {
  XORAllRower aar;
  df->pmap(aar);
  ASSERT_EQ(xor_answer, aar.sum());
}
