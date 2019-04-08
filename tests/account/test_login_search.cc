/*
Author:Liubingqi
date:2019.03.22

g++ UserModelClient.cc -lpistache  -std=c++11 -o userclient
./client http://localhost:9080 3
*/

#include <iostream>
#include "usermodel/Account.h"
//#include <uuid/uuid.h>
#include "common/JsonSerializer.h"
#include "context.h"
#include "datastore/datastore.h"
#include "gtest/gtest.h"


#include <atomic>
#include <pistache/net.h>
#include <pistache/http.h>
#include <pistache/client.h>

using namespace Pistache;
using namespace Pistache::Http;

class HVSAccountTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
  static void SetUpTestCase() { hvs::init_context(); }
  static void TearDownTestCase() { hvs::destroy_context(); }
};

TEST_F(HVSAccountTest, atry) {
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

    std::string mes = "{\"HVSAccountName\":\"lbq\",\"HVSPassword\":\"123456\"}";
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