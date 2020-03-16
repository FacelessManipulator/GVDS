#include <iostream>
#include "gvds_struct.h"
#include "manager/space/SpaceServer.h"
#include "common/JsonSerializer.h"
#include "context.h"
#include "datastore/datastore.h"
#include "common/json.h"
#include "datastore/couchbase_helper.h"
#include "gtest/gtest.h"
#include <pistache/client.h>
#include <atomic>
#include <future>



using namespace gvds;

class GVDSSpaceTest : public ::testing::Test {
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

// TEST_F(GVDSSpaceTest, Simple){
//   std::string tmp_key = "dca31346-91c3-48d1-865a-eef36b314c80";
//   Space tmps;
//   std::shared_ptr<gvds::CouchbaseDatastore> spacePtr = std::make_shared<gvds::CouchbaseDatastore>(
//       gvds::CouchbaseDatastore("space_info"));
//   spacePtr->init();
//   tmps.spaceID = "dca31346-91c3-48d1-865a-eef36b314c80";
//   tmps.spaceName = "beijingspace";
//   tmps.spaceSize = 100;
//   tmps.hostCenterID = "beijing";
//   tmps.spacePath = "/123/beijingspace";
//   std::string tmp_value = tmps.serialize();
//   spacePtr->set(tmp_key, tmp_value);
// }

TEST_F(GVDSSpaceTest, Rename) {
    Http::Client client;
    char url[256];
    snprintf(url, 256, "http://localhost:%d/space/rename", manager->rest_port());
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);

    SpaceRequest req;
    req.spaceID = "965e3ce8-d8a6-492e-accc-02469c24a42c";
    req.newSpaceName = "10000";

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

TEST_F(GVDSSpaceTest, changesize) {
    Http::Client client;
    char url[256];
    snprintf(url, 256, "http://localhost:%d/space/changesize", manager->rest_port());
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);

    SpaceRequest req;
    req.spaceID = "18530785-7f85-4ebd-ab58-75469c03c718";
    req.newSpaceSize = 50;

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


