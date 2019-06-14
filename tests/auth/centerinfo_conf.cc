


//这个文件不用了


//=============
#include <iostream>
#include "common/JsonSerializer.h"
#include "context.h"
#include "datastore/datastore.h"
#include "common/json.h"
#include "datastore/couchbase_helper.h"
#include "gtest/gtest.h"



// #include "manager/usermodel/Account.h"
#include "manager/authmodel/AuthModelServer.h"
#include "common/centerinfo.h"

#include <pistache/client.h>
#include <pistache/http.h>
#include <pistache/net.h>
#include <atomic>

#include "manager/manager.h"

//#include "include/context.h"
using namespace Pistache;
using namespace Pistache::Http;
using namespace hvs;
using namespace std;

class HVSAuthTest : public ::testing::Test {
 protected:
  void SetUp() override {
    manager = static_cast<Manager*>(HvsContext::get_context()->node);
    ASSERT_NE(manager, nullptr);
  }
  void TearDown() override { manager = nullptr; }

 protected:
  static void SetUpTestCase() {
    hvs::init_context();
    hvs::init_manager();
    usleep(100000); // wait 100 ms. rest server may started.
  }
  static void TearDownTestCase() {
    hvs::destroy_manager(
        static_cast<Manager*>(HvsContext::get_context()->node));
    hvs::destroy_context();
  }

 public:
  Manager* manager;
};




TEST_F(HVSAuthTest, centerinfo_conf) {
    
    string info = *(HvsContext::get_context()->_config->get<std::string>("center_information"));
    cout << info << endl;
    CenterInfo mycenter;
    mycenter.deserialize(info);

    cout << mycenter.center1["centerID"] << endl;
    cout << mycenter.center1["centerName"] << endl;
    cout << mycenter.center1["managerIP"] << endl;
    cout << mycenter.center1["managerPort"] << endl;

}