#pragma once
// lang: CwC

#include "../client/application.h"

/** Represents a testing application to ensure all parts are working.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class TestApp : public Application {
public:
  TestApp(size_t idx, Network *net) : Application(idx, net) {}

  void run_() { printf("TestApp ran\n"); }
};