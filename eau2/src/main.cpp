#include "apps/demo.h"
#include "apps/trivial.h"
#include "apps/wordcount.h"

#include "client/application.h"
#include "client/arg.h"
#include "client/network-ip.h"
#include "client/network-pseudo.h"

#include "utils/serializer.h"

Arguments arg;

/** Acquires the correct network given the arguments passed in. **/
Network *get_network() {
  if (arg.pseudo_network) {
    printf("Initializing a pseudo network...\n");
    return new NetworkPseudo(arg.num_nodes);
  } else {
    printf("Initializing IP network...\n");
    NetworkIP *network = new NetworkIP();
    if (arg.index == 0) {
      network->init_server(arg.index, arg.ip, arg.port, arg.num_nodes);
    } else {
      network->init_client(arg.index, arg.ip, arg.port, arg.server_ip,
                           arg.server_port);
    }
    return network;
  }
}

/** Runs applications concurrently via threads. **/
class ApplicationThread : public Thread {
public:
  Application *app = nullptr;
  ~ApplicationThread() { delete app; }
  void run() { app->start(); }
};

/** Select application type. **/
Application *get_app(size_t index, Network *network) {
  if (strcmp(arg.app, "trivial") == 0) {
    return new Trivial(index, network);
  } else if (strcmp(arg.app, "demo") == 0) {
    return new Demo(index, network);
  } else if (strcmp(arg.app, "wc") == 0) {
    return new WordCount(index, network);
  }
  assert(false);
}

int main(int argc, char **argv) {
  arg.parse(argc, argv);
  assert(arg.num_nodes != 0); // Ensure there is at least one node

  Network *network = get_network();
  if (arg.pseudo_network) {
    ApplicationThread *threads = new ApplicationThread[arg.num_nodes];
    for (size_t i = 0; i < arg.num_nodes; i++) {
      threads[i].app = get_app(i, network);
      threads[i].start();
    }
    for (size_t i = 0; i < arg.num_nodes; i++) {
      threads[i].join();
    }
    delete[] threads;
  } else {
    Application *app = get_app(arg.index, network);
    app->start();
    delete app;
  }

  delete network;
}