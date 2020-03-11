#include "store/dataframe.h"

int main() {
  Schema s("S");
  DataFrame df(s);

  Row r(s);
  for (size_t i = 0; i < 10; i++) {
    r.set(0, new String("Foobar"));
    df.add_row(r);
  }

  String fb("Foobar");
  assert(fb.equals(df.get_string(0, 9)));
}
