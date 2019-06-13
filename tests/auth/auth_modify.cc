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
 #include "manager/authmodel/Auth.h"

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


TEST_F(HVSAuthTest, auth_modify) {
    Http::Client client;
    char url[256];
    //snprintf(url, 256, "http://localhost:%d/auth/modify", manager->rest_port());
    snprintf(url, 256, "http://localhost:9090/auth/modify");
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);

    FEAuthModifygroupinfo FEgroup;
    FEgroup.hvsID = "127";
    FEgroup.zonename = "zone-auth-test1";
    FEgroup.modify_groupauth = "0";

    string value = FEgroup.serialize();

    std::cout << "before" << endl;
    auto response = client.post(url).cookie(Http::Cookie("FOO", "bar")).body(value).send();
            dout(-1) << "Client Info: post request " << url << dendl;

    std::cout << "after" << endl;
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