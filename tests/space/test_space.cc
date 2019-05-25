#include <iostream>
#include "manager/space/Space.h"
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

// TEST_F(HVSSpaceTest, Simple){
//   std::string tmp_key = "dca31346-91c3-48d1-865a-eef36b314c80";
//   Space tmps;
//   std::shared_ptr<hvs::CouchbaseDatastore> spacePtr = std::make_shared<hvs::CouchbaseDatastore>(
//       hvs::CouchbaseDatastore("space_info"));
//   spacePtr->init();
//   tmps.spaceID = "dca31346-91c3-48d1-865a-eef36b314c80";
//   tmps.spaceName = "beijingspace";
//   tmps.spaceSize = 100;
//   tmps.hostCenterID = "beijing";
//   tmps.spacePath = "/123/beijingspace";
//   std::string tmp_value = tmps.serialize();
//   spacePtr->set(tmp_key, tmp_value);
// }

TEST_F(HVSSpaceTest, Rename) {
  Http::Client client;
  char url[256];
  snprintf(url, 256, "http://localhost:%d/space/rename", manager->rest_port());
  auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
  client.init(opts);

  SpaceRenameReq req;
  req.spaceID = "dca31346-91c3-48d1-865a-eef36b314c80";
  req.newSpaceName = "superxu";

  std::string value = req.serialize();

  auto response = client.post(url).body(value).send();
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

TEST_F(HVSSpaceTest, changesize) {
    Http::Client client;
    char url[256];
    snprintf(url, 256, "http://localhost:%d/space/changesize", manager->rest_port());
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);

    SpaceSizeChangeReq req;
    req.spaceID = "dca31346-91c3-48d1-865a-eef36b314c80";
    req.newSpaceSize = 100;

    std::string value = req.serialize();

    auto response = client.post(url).body(value).send();
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


