#include <iostream>
#include "usermodel/Account.h"
//#include <uuid/uuid.h>
#include "common/JsonSerializer.h"
#include "common/RestServer.h"
#include "context.h"
#include "datastore/datastore.h"
#include "gtest/gtest.h"

class HVSRestTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
  static void SetUpTestCase() { hvs::init_context(); }
  static void TearDownTestCase() { hvs::destroy_context(); }
};

TEST_F(HVSRestTest, atry) {
  cout << "HVSRestTest Begin:" << endl;
/*
  auto p = init_rest();
  sleep(1000);
  stop_rest(p);
*/

  sleep(1000);
 

  // EXPECT_TRUE(lbqprint()==0);
}
