#include "utils/serializer.h"
#include "utils/string.h"

int main() {
  String *s = new String("Apple juice");

  Serializer ser;
  s->serialize(ser);

  Deserializer dser(*ser.data());
  String *q = String::deserialize(dser);

  assert(s->equals(q));

  delete s;
  delete q;
}
