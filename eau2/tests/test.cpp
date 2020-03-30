#include <gtest/gtest.h>

#include "test-array.h"
#include "test-column.h"
#include "test-dataframe.h"
#include "test-map.h"
#include "test-object.h"
#include "test-pmap.h"
#include "test-queue.h"
#include "test-row.h"
#include "test-rower.h"
#include "test-schema.h"
#include "test-serializer.h"
#include "test-string.h"
#include "test-util.h"

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
