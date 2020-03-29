#pragma once
// lang: CwC

#include "../utils/string.h"

static void usage(const char *arg0) {
  fprintf(stderr,
          "Usage: %s [-ip IPV4_ADDRESS] [-port PORT_NUM] "
          "[-server_ip IPV4_ADDRESS] [-server_port PORT_NUM] "
          "[-index NUMBER] [-nodes NUMBER] [-app APP_NAME]\n"
          "Example: %s -ip 102.168.0.1\n"
          "         %s -ip 192.168.1.1 -port 8080\n",
          arg0, arg0, arg0);
  exit(0);
}

/** Represents the arguments to the program.
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu **/
class Arguments : public Object {
public:
  size_t index = 0;
  size_t num_nodes = 0;
  String *ip = nullptr;
  size_t port = 8080;
  String *server_ip = nullptr;
  size_t server_port = 8080;
  bool pseudo_network = false;
  bool is_server = true;
  char *app = nullptr;

  void parse(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
      if (eq_("-h", argv[i])) {
        usage(argv[0]);
      } else if (eq_("-ip", argv[i]) && i + 1 < argc) {
        ip = new String(argv[i + 1]);
      } else if (eq_("-port", argv[i]) && i + 1 < argc) {
        port = strtoul(argv[i + 1], NULL, 10);
      } else if (eq_("-server_ip", argv[i]) && i + 1 < argc) {
        is_server = false;
        server_ip = new String(argv[i + 1]);
      } else if (eq_("-server_port", argv[i]) && i + 1 < argc) {
        is_server = false;
        server_port = strtoul(argv[i + 1], NULL, 10);
      } else if (eq_("-index", argv[i]) && i + 1 < argc) {
        index = strtoul(argv[i + 1], NULL, 10);
      } else if (eq_("-num_nodes", argv[i]) && i + 1 < argc) {
        num_nodes = strtoul(argv[i + 1], NULL, 10);
      } else if (eq_("-pseudo", argv[i])) {
        pseudo_network = true;
      } else if (eq_("-app", argv[i]) && i + 1 < argc) {
        app = argv[i + 1];
      }
    }
  }

  bool eq_(const char *a, const char *b) { return strcmp(a, b) == 0; }
};

extern Arguments arg;