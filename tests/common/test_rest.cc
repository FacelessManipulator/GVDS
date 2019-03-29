#include "usermodel/Account.h"
#include <iostream>
//#include <uuid/uuid.h>
#include "common/JsonSerializer.h"
#include "datastore/datastore.h"
#include "common/RestServer.h"
#include "context.h"
#include "gtest/gtest.h"

class HVSRestTest : public ::testing::Test {
 protected:
  void SetUp() override { hvs::init_context(); }
  void TearDown() override { hvs::destroy_context(); }
};


TEST_F(HVSRestTest, atry){
    cout << "HVSRestTest Begin:"<< endl;
    auto rest = init_rest();
    stop_rest(rest);
    
    //EXPECT_TRUE(lbqprint()==0);
}
