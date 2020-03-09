// lang: CwC
#pragma once
#include <gtest/gtest.h>

/**
 * @brief Here are some macros that mimic C's assert.h library.
 *
 * @author griep.p@husky.neu.edu & colabella.a@husky.neu.edu
 */
#define ASSERT_EXIT_ZERO(a)                                                    \
  ASSERT_EXIT(a(), ::testing::ExitedWithCode(0), ".*");
#define ASSERT_FAIL(a) ASSERT_DEATH(a, "");
#define ASSERT(a) ASSERT_EQ(a, true);
#define ASSERT_SEQ(a, b) ASSERT_EQ(strcmp(a, b), 0);
