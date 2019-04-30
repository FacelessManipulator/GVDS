/*
Author:Liubingqi
date:2019.03.22

g++ UserModelClient.cc -lpistache  -std=c++11 -o userclient
./client http://localhost:9080 3
*/

#include <iostream>
#include "aggregation/StorageResBicInfo.h"
#include "common/JsonSerializer.h"
#include "context.h"
#include "datastore/datastore.h"
#include "gtest/gtest.h"

#include <pistache/client.h>
#include <pistache/http.h>
#include <pistache/net.h>
#include <atomic>

using namespace Pistache;
using namespace Pistache::Http;
using namespace hvs;

class HVSAggregateTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
  static void SetUpTestCase() {
    hvs::init_context();
    port = HvsContext::get_context()->_rest->getPort();
  }
  static void TearDownTestCase() { hvs::destroy_context(); }

 public:
  static int port;
};

int HVSAggregateTest::port = 0;

TEST_F(HVSAggregateTest, registration) {
  cout << "******start resource aggregate: register ******" << endl;

  // 第二个参数传地址 第三个参数传请求数量 默认1
  char page_url[128];
  snprintf(page_url, 128, "http://localhost:%d/resource/register", port);
  std::string page(page_url); // /resource/logout
  int count = 1;

  Http::Client client;

  auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
  client.init(opts);

  std::vector<Async::Promise<Http::Response>> responses;

  std::atomic<size_t> completedRequests(0);
  std::atomic<size_t> failedRequests(0);

  //计时
  auto start = std::chrono::steady_clock::now();

  std::shared_ptr<StorageResBicInfo> resInfoPtr(
      new StorageResBicInfo("1", "Beijing", "center_1", "zhongkeyuan", 1024,
                            "http://193.195.34.65", Normal));
  std::string value = resInfoPtr->serialize();

  auto resp =
      client.post(page).cookie(Http::Cookie("FOO", "bar")).body(value).send();
  resp.then(
      [&](Http::Response response) {
        ++completedRequests;
        std::cout << "Response code = " << response.code() << std::endl;
        // response body
        auto body = response.body();
        if (!body.empty()) {
          std::cout << "Response body = " << body << std::endl;
        }
      },
      Async::IgnoreException);
  responses.push_back(std::move(resp));

  auto sync = Async::whenAll(responses.begin(), responses.end());
  Async::Barrier<std::vector<Http::Response>> barrier(sync);
  barrier.wait_for(std::chrono::seconds(5));

  auto end = std::chrono::steady_clock::now();
  std::cout << "Summary of execution" << std::endl
            << "Total number of requests sent     : " << count << std::endl
            << "Total number of responses received: "
            << completedRequests.load() << std::endl
            << "Total number of requests failed   : " << failedRequests.load()
            << std::endl
            << "Total time of execution           : "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     start)
                   .count()
            << "ms" << std::endl;

  client.shutdown();
  cout << "******endl client: storage resource registration ******" << endl;
}

TEST_F(HVSAggregateTest, logout) {
  cout << "******start resource aggregate: logout ******" << endl;

  // 第二个参数传地址 第三个参数传请求数量 默认1
  char page_url[128];
  snprintf(page_url, 128, "http://localhost:%d/resource/logout", port);
  std::string page(page_url); // /resource/logout
  int count = 1;

  Http::Client client;

  auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
  client.init(opts);

  std::vector<Async::Promise<Http::Response>> responses;

  std::atomic<size_t> completedRequests(0);
  std::atomic<size_t> failedRequests(0);

  //计时
  auto start = std::chrono::steady_clock::now();

  std::shared_ptr<StorageResBicInfo> resInfoPtr(
      new StorageResBicInfo("1", "Beijing", "center_1", "zhongkeyuan", 1024,
                            "http://193.195.34.65", Normal));
  std::string value = resInfoPtr->serialize();

  auto resp =
      client.post(page).cookie(Http::Cookie("FOO", "bar")).body(value).send();
  resp.then(
      [&](Http::Response response) {
        ++completedRequests;
        std::cout << "Response code = " << response.code() << std::endl;
        // response body
        auto body = response.body();
        if (!body.empty()) {
          std::cout << "Response body = " << body << std::endl;
        }
      },
      Async::IgnoreException);
  responses.push_back(std::move(resp));

  auto sync = Async::whenAll(responses.begin(), responses.end());
  Async::Barrier<std::vector<Http::Response>> barrier(sync);
  barrier.wait_for(std::chrono::seconds(5));

  auto end = std::chrono::steady_clock::now();
  std::cout << "Summary of execution" << std::endl
            << "Total number of requests sent     : " << count << std::endl
            << "Total number of responses received: "
            << completedRequests.load() << std::endl
            << "Total number of requests failed   : " << failedRequests.load()
            << std::endl
            << "Total time of execution           : "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     start)
                   .count()
            << "ms" << std::endl;

  client.shutdown();

  cout << "******endl client: storage resource logout ******" << endl;
}
