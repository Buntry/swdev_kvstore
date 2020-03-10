#include "test-array.h"
#include "test-column.h"
#include "test-dataframe.h"
#include "test-object.h"
#include "test-pack.h"
#include "test-pmap.h"
#include "test-row.h"
#include "test-rower.h"
#include "test-schema.h"
#include "test-serial.h"
#include "test-string.h"
#include <gtest/gtest.h>

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
