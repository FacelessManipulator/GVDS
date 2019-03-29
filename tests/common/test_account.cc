#include "usermodel/Account.h"
#include <iostream>
//#include <uuid/uuid.h>
#include "common/JsonSerializer.h"
#include "datastore/datastore.h"
#include "context.h"
#include "gtest/gtest.h"

class HVSAccountTest : public ::testing::Test {
 protected:
  void SetUp() override { hvs::init_context(); }
  void TearDown() override { hvs::destroy_context(); }
};


TEST_F(HVSAccountTest, atry){
    EXPECT_TRUE(lbqprint()==0);
}
