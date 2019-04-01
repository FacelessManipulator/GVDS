#include <iostream>
#include "usermodel/Account.h"
//#include <uuid/uuid.h>
#include "common/JsonSerializer.h"
#include "context.h"
#include "datastore/datastore.h"
#include "gtest/gtest.h"

class HVSAccountTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
  static void SetUpTestCase() { hvs::init_context(); }
  static void TearDownTestCase() { hvs::destroy_context(); }
};

TEST_F(HVSAccountTest, atry) {
  //  EXPECT_TRUE(lbqprint()==0);
}
