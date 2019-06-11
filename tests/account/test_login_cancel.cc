/*
Author:Liubingqi
date:2019.03.22

g++ UserModelClient.cc -lpistache  -std=c++11 -o userclient
./client http://localhost:9080 3
*/

//=============
#include <iostream>
#include "common/JsonSerializer.h"
#include "context.h"
#include "datastore/datastore.h"
#include "common/json.h"
#include "datastore/couchbase_helper.h"
#include "gtest/gtest.h"



// #include "manager/usermodel/Account.h"
// #include "manager/usermodel/UserModelServer.h"

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


TEST_F(HVSAccountTest, login_cancel) {
  Http::Client client;
  char url[256];
  snprintf(url, 256, "http://localhost:%d/users/login", manager->rest_port());
  auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
  client.init(opts);

  
  std::string mes = "{\"HVSAccountName\":\"lbq-7\",\"HVSPassword\":\"123456\"}";
  std::string mtoken;


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
        std::cout<< "Response cookie = ";
        auto cookies = res.cookies();
        for (const auto& c: cookies) {
            std::cout << c.name << " : " << c.value << std::endl;
            mtoken = c.value;
        }
        prom.set_value(true);
      },
      Async::IgnoreException);
  EXPECT_TRUE(fu.get());

  sleep(5);
  //cancel
  snprintf(url, 256, "http://localhost:%d/users/cancel/127", manager->rest_port());
  auto response_1 = client.get(url).cookie(Http::Cookie("token", mtoken)).send();
        dout(-1) << "Client Info: get request " << url << dendl;

  std::promise<bool> prom_1;
  auto fu_1 = prom_1.get_future();
  response_1.then(
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
        prom_1.set_value(true);
      },
      Async::IgnoreException);
  EXPECT_TRUE(fu_1.get());

  client.shutdown();
}
