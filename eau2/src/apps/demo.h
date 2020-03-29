#pragma once
// lang: CwC

#include "../client/application.h"

/** Represents a minimum-viable product of the application.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Demo : public Application {
public:
  Key *main = new Key("main", 0);
  Key *verify = new Key("verif", 0);
  Key *check = new Key("ck", 0);

  Demo(size_t idx, Network *network) : Application(idx, network) {}
  ~Demo() {
    delete main;
    delete verify;
    delete check;
  }

  /** Assigns each node in the network a different job. **/
  void run_() override {
    switch (this_node()) {
    case 0:
      producer();
      break;
    case 1:
      counter();
      break;
    case 2:
      summarizer();
    }
  }

  /** Generates an array of numbers and computes the expected sum. **/
  void producer() {
    size_t SZ = 100 * 1000;
    float *vals = new float[SZ];
    float sum = 0;
    for (size_t i = 0; i < SZ; ++i)
      sum += vals[i] = i;
    DataFrame::fromArray(main, this_store(), SZ, vals);
    DataFrame::fromScalar(check, this_store(), sum);
    delete[] vals;
  }

  /** Reads in an array of numbers and computes the actual sum. **/
  void counter() {
    DataFrame *v = this_store()->get_and_wait(main);
    size_t sum = 0;
    for (size_t i = 0; i < 100 * 1000; ++i)
      sum += v->get_float(0, i);
    p("The sum is  ").pln(sum);
    DataFrame::fromScalar(verify, this_store(), sum);
  }

  /** Fetches both the actual sum and expected sum and checks them.  **/
  void summarizer() {
    DataFrame *result = this_store()->get_and_wait(verify);
    DataFrame *expected = this_store()->get_and_wait(check);
    pln(expected->get_float(0, 0) == result->get_float(0, 0) ? "SUCCESS"
                                                             : "FAILURE");
  }
};