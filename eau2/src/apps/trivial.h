#pragma once
// lang: CwC

#include "../client/application.h"

/** Represents a Trivial application
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Trivial : public Application {
public:
  Trivial(size_t idx, Network *net) : Application(idx, net) {}
  void run_() {

    // Initialize the vals array and sum
    size_t SZ = 1000 * 1000;
    float *vals = new float[SZ];
    float sum = 0;
    for (size_t i = 0; i < SZ; i++) {
      sum += vals[i] = i;
    }

    Key key("triv", 0);
    DataFrame *df = DataFrame::fromArray(&key, this_store(), SZ, vals);

    /** Load the data from the store. **/
    DataFrame *df2 = this_store()->get(&key);
    float should_be_same_sum = 0;
    for (size_t i = 0; i < SZ; ++i) {
      should_be_same_sum += df2->get_float(0, i);
    }

    assert(sum == should_be_same_sum);

    delete[] vals;
    delete df;
    delete df2;
  }
};