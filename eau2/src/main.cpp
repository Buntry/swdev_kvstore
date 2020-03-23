#include "store/dataframe.h"
#include "utils/serializer.h"

int main() {
  Schema s("IS");
  DataFrame df(s);

  String *q = new String("Foobar");

  Row r(s);
  for (size_t i = 0; i < 3; i++) {
    r.set(0, (int)i);
    r.set(1, q->clone());
    df.add_row(r);
  }

  for (size_t i = 0; i < 3; i++) {
    df.fill_row(i, r);
    assert((int)i == r.get_int(0));
  }

  DataFrame *df2 = df.clone();
  std::cout << df2->ncols() << std::endl;
  assert(df2->ncols() == 2);
  assert(df2->nrows() == 3);

  df2->fill_row(0, r);
  assert(r.get_int(0) == (int)0);
  df2->fill_row(1, r);
  assert(r.get_int(0) == (int)1);
  df2->fill_row(2, r);
  assert(r.get_int(0) == (int)2);
  assert(r.get_string(1)->equals(q));

  delete q;
  delete df2;
}