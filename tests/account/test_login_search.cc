/*
Author:Liubingqi
date:2019.03.22

g++ UserModelClient.cc -lpistache  -std=c++11 -o userclient
./client http://localhost:9080 3
*/

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


TEST_F(GVDSAccountTest, login_search) {
  Http::Client client;
  char url[256];
  snprintf(url, 256, "http://localhost:%d/users/login", manager->rest_port());
  auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
  client.init(opts);

  
  std::string mes = "{\"GVDSAccountName\":\"lbq-7\",\"GVDSPassword\":\"123456\"}";
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
  //search  
  snprintf(url, 256, "http://localhost:%d/users/search/127", manager->rest_port());
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

  // tmp
  //UserModelServer *p_usermodel = static_cast<UserModelServer*>(mgr->get_module("user").get());
  //string value = p_usermodel->getLocalAccountinfo("127", "Shanghai");
  
  // cout << "=====*****============" << endl;
  // UserModelServer tmp_u;
  // string value = tmp_u.getLocalAccountinfo("127", "Jinan");
  // cout << "=====add account map" <<value << endl;
  //

  client.shutdown();
}

/*
TEST_F(GVDSAccountTest, atry) {
    cout<< "****** start client: login_search ******"<<endl;

    // 第二个参数传地址 第三个参数传请求数量 默认1
    int count = 2;
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

    std::string mes = "{\"GVDSAccountName\":\"lbq\",\"GVDSPassword\":\"123456\"}";
    std::string mtoken;
    
    auto resp = client.post("http://localhost:9080/users/login").cookie(Http::Cookie("FOO", "bar")).body(mes).send();

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
        std::cout<< "Response cookie = ";
        auto cookies = response.cookies();
        for (const auto& c: cookies) {
            std::cout << c.name << " : " << c.value << std::endl;
            mtoken = c.value;
        }
    }, Async::IgnoreException);
    responses.push_back(std::move(resp));

    auto sync_login = Async::whenAll(responses.begin(), responses.end());
    Async::Barrier<std::vector<Http::Response>> barrier_login(sync_login);

    std::cout << "start barrier_login.wait_for"<<std::endl;
    barrier_login.wait_for(std::chrono::seconds(5));
    std::cout << "end barrier_login.wait_for"<<std::endl;

    //search
    auto resp_1 = client.get("http://localhost:9080/users/search/78910").cookie(Http::Cookie("token", mtoken)).send();

    resp_1.then([&](Http::Response response) {
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
    responses.push_back(std::move(resp_1));


    auto sync = Async::whenAll(responses.begin(), responses.end());
    Async::Barrier<std::vector<Http::Response>> barrier(sync);

    std::cout << "start barrier.wait_for"<<std::endl;
    barrier.wait_for(std::chrono::seconds(5));
    std::cout << "end barrier.wait_for"<<std::endl;

    auto end = std::chrono::steady_clock::now();
    std::cout << "Summary of execution" << std::endl
              << "Total number of requests sent     : " << count << std::endl
              << "Total number of responses received: " << completedRequests.load() << std::endl
              << "Total number of requests failed   : " << failedRequests.load() << std::endl
              << "Total time of execution           : "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    client.shutdown();

    cout<< "****** end client: login_search ******"<<endl;
}
*/