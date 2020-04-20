//=============
#include <iostream>
#include "common/JsonSerializer.h"
#include "gvds_context.h"
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
#include <future>

//#include "include/context.h"
using namespace Pistache;
using namespace Pistache::Http;
using namespace gvds;
using namespace std;

class GVDSAccountTest : public ::testing::Test {
 protected:
  void SetUp() override {
    manager = static_cast<Manager*>(HvsContext::get_context()->node);
    ASSERT_NE(manager, nullptr);
  }
  void TearDown() override { manager = nullptr; }

 protected:
  static void SetUpTestCase() {
    gvds::init_context();
    gvds::init_manager();
    usleep(100000); // wait 100 ms. rest server may started.
  }
  static void TearDownTestCase() {
    gvds::destroy_manager(
        static_cast<Manager*>(HvsContext::get_context()->node));
    gvds::destroy_context();
  }

 public:
  Manager* manager;
};


TEST_F(GVDSAccountTest, test_mconf_search) {
  Http::Client client;
  char url[256];
  snprintf(url, 256, "http://localhost:9090/mconf/searchCenter");
  auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
  client.init(opts);

  cout << "here1" << endl;
  auto response = client.get(url).cookie(Http::Cookie("FOO", "bar")).send();
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
            if(body!="fail"){
                //====================
                //your code write here
                cout << "can get here" << endl;
                CenterInfo mycenter;
                mycenter.deserialize(body);   

                vector<string>::iterator iter;
                for( iter = mycenter.centerID.begin(); iter!=mycenter.centerID.end(); iter++){
                  cout << "centerID: "<< *iter << endl;
                  cout << "centerIP: "<< mycenter.centerIP[*iter] << endl;
                  cout << "centerPort: "<< mycenter.centerPort[*iter] << endl;
                  cout << "centerName: "<< mycenter.centerName[*iter] << endl;
                }
              //====================
            }
        }
        prom.set_value(true);
      },
      Async::IgnoreException);
  EXPECT_TRUE(fu.get());

  client.shutdown();
}

