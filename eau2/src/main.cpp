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

int main(int argc, char **argv) {
  arg.parse(argc, argv);
  assert(arg.num_nodes != 0); // Ensure there is at least one node

  Network *network = get_network();

  delete network;
}