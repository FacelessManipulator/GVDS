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



#include "manager/usermodel/Account.h"
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




TEST_F(HVSAccountTest, registration) {
    cout<< "******start client: registration ******"<<endl;
    Http::Client client;
    char url[256];
    snprintf(url, 256, "http://localhost:%d/users/registration", manager->rest_port());

    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);

    Account person("lbq-8", "123456", "128", "XXXXXX@163.com", "15012349876", "xueyuanlu",  "Beihang");
    std::string value = person.serialize();

    auto response = client.post(url).cookie(Http::Cookie("FOO", "bar")).body(value).send();
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


/*
TEST_F(HVSAccountTest, atry) {
    cout<< "******start client: registration ******"<<endl;

    // 第二个参数传地址 第三个参数传请求数量 默认1
    std::string page = "http://localhost:9080/users/registration";
    int count = 1;

    Http::Client client;

    auto opts = Http::Client::options()
        .threads(1)
        .maxConnectionsPerHost(8);
    client.init(opts);

    std::vector<Async::Promise<Http::Response>> responses;

    std::atomic<size_t> completedRequests(0);
    std::atomic<size_t> failedRequests(0);

    //计时
    auto start = std::chrono::steady_clock::now();

    Account person("lbq-2", "123456", "111213", "XXXXXX@163.com", "15012349876", "xueyuanlu",  "Beihang", "has");
    person.sc.location_scacc["beijing"] = "local_lbq";
    person.sc.scacc_password["local_lbq"] = "654321";
    person.sc.location_scacc["shanghai"] = "local_lbq1";
    person.sc.scacc_password["local_lbq1"] = "654321";

    std::string value = person.serialize();

    auto resp = client.post(page).cookie(Http::Cookie("FOO", "bar")).body(value).send();
    resp.then([&](Http::Response response) {
            ++completedRequests;
        std::cout << "Response code = " << response.code() << std::endl;
        //response body
        auto body = response.body();
        if (!body.empty()){
            std::cout << "Response body = " << body << std::endl;
            //====================
            //your code write here

            //====================
        }
    }, Async::IgnoreException);
    responses.push_back(std::move(resp));

    auto sync = Async::whenAll(responses.begin(), responses.end());
    Async::Barrier<std::vector<Http::Response>> barrier(sync);
    barrier.wait_for(std::chrono::seconds(5));

    auto end = std::chrono::steady_clock::now();
    std::cout << "Summary of execution" << std::endl
              << "Total number of requests sent     : " << count << std::endl
              << "Total number of responses received: " << completedRequests.load() << std::endl
              << "Total number of requests failed   : " << failedRequests.load() << std::endl
              << "Total time of execution           : "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    client.shutdown();

    cout<< "******endl client: registration ******"<<endl;
  
}
*/