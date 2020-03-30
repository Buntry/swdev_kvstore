#pragma once
// lang: CwC

#include "../client/application.h"

/** Represents a testing application to ensure all parts are working.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class TestApp : public Application {
public:
  TestApp(size_t idx, Network *net) : Application(idx, net) {}

  /** Plays a game of telephone **/
  void run_() {
    printf("Test App running @ index %d.\n", (int)this_node());
    if (arg.num_nodes <= 1)
      return;

    if (this_node() == 0) {
      Thread::sleep(200);
      Message *msg = new Status(new String("Telephone!"));
      msg->init(this_node(), target(), this_node());
      network()->send_msg(msg);
    } else if (this_node() < arg.num_nodes - 1) {
      Status *msg = dynamic_cast<Status *>(network()->receive_msg());
      msg->init(this_node(), target(), this_node());
      network()->send_msg(msg);
    } else {
      Status *msg = dynamic_cast<Status *>(network()->receive_msg());
      printf("TestApp %d got message: %s\n", (int)this_node(),
             msg->s()->c_str());
      delete msg;
    }
  }

  /** Gets the next person in line. **/
  size_t target() { return (this_node() + 1) % arg.num_nodes; }
};