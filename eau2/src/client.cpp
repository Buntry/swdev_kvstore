#include "network.h"

int main(int argc, char **argv) {
  assert(argc >= 9 && strcmp(argv[1], "-ip") == 0);
  assert(strcmp(argv[3], "-p") == 0);
  assert(strcmp(argv[5], "-sip") == 0);
  assert(strcmp(argv[7], "-sp") == 0);
  Node n(argv[2], atoi(argv[4]));

  n.register_with(argv[6], atoi(argv[8]));
  n.service();
}
