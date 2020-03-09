#include "network.h"

int main(int argc, char **argv) {
  assert(argc >= 3 && strcmp(argv[1], "-ip") == 0);
  size_t n_waitfor = 1;
  if (argc >= 5 && strcmp(argv[3], "-n") == 0) {
    n_waitfor = atoi(argv[4]);
  }

  Node n(argv[2], 8080, n_waitfor);
  n.service();
}
