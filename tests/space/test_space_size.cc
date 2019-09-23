//
// Created by root on 9/16/19.
//

#include <iostream>
#include "hvs_struct.h"
#include "manager/space/SpaceServer.h"
#include "common/JsonSerializer.h"
#include "context.h"
#include "datastore/datastore.h"
#include "common/json.h"
#include "datastore/couchbase_helper.h"
#include "gtest/gtest.h"
#include <pistache/client.h>
#include <atomic>

using namespace hvs;

class HVSSpaceTest : public ::testing::Test {
 protected:
  void SetUp() override {
//    manager = static_cast<Manager*>(HvsContext::get_context()->node);
//    ASSERT_NE(manager, nullptr);
  }
  void TearDown() override { /*manager = nullptr;*/ }

 protected:
  static void SetUpTestCase() {
    hvs::init_context();
//    hvs::init_manager();
//    usleep(100000); // wait 100 ms. rest server may started.
  }
  static void TearDownTestCase() {
//    hvs::destroy_manager(
//        static_cast<Manager*>(HvsContext::get_context()->node));
    hvs::destroy_context();
  }

 public:
//  Manager* manager;
};

TEST_F(HVSSpaceTest, SpaceUsageCheckRest) {
    Http::Client client;
    char url[256];
    snprintf(url, 256, "http://localhost:%d/space/spaceusagecheck", 9090);
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);
    SpaceRequest req;
    req.type = SpaceRequest::SpaceRequestType::usage;
    req.spaceIDs.emplace_back("948d0824-b0de-4e0b-a7ee-1b83256932ba");

    auto response = client.post(url).body(req.serialize()).send();
        dout(-1) << "Client Info: post request " << url << dendl;

    std::promise<bool> prom;
    auto fu = prom.get_future();
    response.then(
      [&](Http::Response res) {
        dout(-1) << "Manager Info: " << res.body() << dendl;
        prom.set_value(true);
      },
      Async::IgnoreException);
    EXPECT_TRUE(fu.get());
    client.shutdown();
}

TEST_F(HVSSpaceTest, Sizecheck) {
    Http::Client client;
    char url[256];
    snprintf(url, 256, "http://localhost:%d/space/spaceusage", 9090);
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);

    std::string path = "size80M";

    auto response = client.post(url).body(path).send();
        dout(-1) << "Client Info: post request " << url << dendl;

    std::promise<bool> prom;
    auto fu = prom.get_future();
    response.then(
      [&](Http::Response res) {
        dout(-1) << "Manager Info: " << res.body() << "KB" << dendl;
        prom.set_value(true);
      },
      Async::IgnoreException);
    EXPECT_TRUE(fu.get());
    client.shutdown();
}


