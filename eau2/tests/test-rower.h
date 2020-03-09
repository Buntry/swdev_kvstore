#include "../src/dataframe.h"
#include "test-macros.h"
#include <gtest/gtest.h>

/**
 * @brief Here's are our unit tests for rowers.
 *
 * @author griep.p@husky.neu.edu, colabella.a@husky.neu.edu
 */
class RowerTest : public ::testing::Test {
public:
  /** Rower **/
  Rower *rr;

  /** Row **/
  Row *r;

  /** Schema **/
  Schema *s1;

  /** Initializes everything **/
  void SetUp() {
    s1 = new Schema("I");
    r = new Row(*s1);
    rr = new Rower();
  }

  /** Tears everything down **/
  void TearDown() {
    delete rr;
    delete r;
    delete s1;
  }
};

TEST_F(RowerTest, Accept) { ASSERT_FALSE(rr->accept(*r)); }
