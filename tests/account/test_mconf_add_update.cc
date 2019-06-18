
//=============
#include <iostream>
#include "common/JsonSerializer.h"
#include "context.h"
#include "datastore/datastore.h"
#include "common/json.h"
#include "datastore/couchbase_helper.h"
#include "gtest/gtest.h"



// #include "manager/usermodel/Account.h"
#include "manager/usermodel/UserModelServer.h"

#include <pistache/client.h>
#include <pistache/http.h>
#include <pistache/net.h>
#include <atomic>

#include "manager/manager.h"
#include "common/centerinfo.h"

//#include "include/context.h"
using namespace Pistache;
using namespace Pistache::Http;
using namespace hvs;
using namespace std;

class HVSAccountTest : public ::testing::Test {
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


TEST_F(HVSAccountTest, test_mconf_add_update) {
  Http::Client client;
  char url[256];
  snprintf(url, 256, "http://localhost:9090/mconf/addCenter");
  auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
  client.init(opts);

  FECenterInfo FEcenter;
  FEcenter.centerID = "4";
  FEcenter.centerIP = "127.0.0.9";
  FEcenter.centerPort = "9090";
  FEcenter.centerName = "Jinan";

  string mes = FEcenter.serialize();
  cout << "mes: " << mes << endl;
  auto response = client.post(url).cookie(Http::Cookie("FOO", "bar")).body(mes).send();
        dout(-1) << "Client Info: post request " << url << dendl;

  std::promise<bool> prom;
  auto fu = prom.get_future();
  response.then(
      [&](Http::Response res) {
        //dout(-1) << "Manager Info: " << res.body() << dendl;
        std::cout << "Response code = " << res.code() << std::endl;
        auto body = res.body();
        if (!body.empty()){
            std::cout << "Response body = " << body << std::endl;
            //====================
            //your code write here

            //====================
        }
        prom.set_value(true);
      },
      Async::IgnoreException);
  EXPECT_TRUE(fu.get());

  client.shutdown();
}