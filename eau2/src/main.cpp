#include "utils/queue.h"

int main() {
  String s("Charlene");
  StringQueue sq;

  for (size_t i = 0; i < 100 * 1000; i++) {
    sq.push(&s);
    sq.push(&s);
    sq.push(&s);
    assert(s.equals(sq.pop()));
  }
}
