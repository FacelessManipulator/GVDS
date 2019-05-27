#include <iostream>
#include "manager/zone/Zone.h"
#include "manager/zone/ZoneServer.h"
#include "common/JsonSerializer.h"
#include "context.h"
#include "datastore/datastore.h"
#include "common/json.h"
#include "datastore/couchbase_helper.h"
#include "gtest/gtest.h"
#include <pistache/client.h>
#include <atomic>



using namespace hvs;

class HVSZoneTest : public ::testing::Test {
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

TEST_F(HVSZoneTest, Rename) {
//    Http::Client client;
//    char url[256];
//    snprintf(url, 256, "http://localhost:%d/zone/rename", manager->rest_port());
//    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
//    client.init(opts);
//
//    ZoneRenameReq req;
//    req.zoneID = "21219e04-f2a2-4d8b-80db-a254caaf9d7e";
//    req.ownerID = "128";
//    req.newZoneName = "superzone";
//
//    std::string value = req.serialize();
//
//    auto response = client.post(url).body(value).send();
//        dout(-1) << "Client Info: post request " << url << dendl;
//
//    std::promise<bool> prom;
//    auto fu = prom.get_future();
//    response.then(
//        [&](Http::Response res) {
//        dout(-1) << "Manager Info: " << res.body() << dendl;
//        prom.set_value(true);
//        },
//        Async::IgnoreException);
//    EXPECT_TRUE(fu.get());
//    client.shutdown();
}

 TEST_F(HVSZoneTest, Locate) {
//     Http::Client client;
//     char url[256];
//     snprintf(url, 256, "http://localhost:%d/zone/locate", manager->rest_port());
//     auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
//     client.init(opts);
//
//     GetZoneLocateInfoReq req;
//     req.clientID = "123";
//     req.zoneID = "21219e04-f2a2-4d8b-80db-a254caaf9d7e";
//     req.spaceID.emplace_back("62621dea-bd6b-4c12-b1ce-5b466eea69af");
//
//     std::string value = req.serialize();
//
//     auto response = client.post(url).body(value).send();
//         dout(-1) << "Client Info: post request " << url << dendl;
//
//     std::promise<bool> prom;
//     auto fu = prom.get_future();
//     response.then(
//         [&](Http::Response res) {
//         dout(-1) << "Manager Info: " << res.body() << dendl;
//         prom.set_value(true);
//         },
//         Async::IgnoreException);
//     EXPECT_TRUE(fu.get());
//     client.shutdown();
 }

 TEST_F(HVSZoneTest, GetInfo) {
//     Http::Client client;
//     char url[256];
//     snprintf(url, 256, "http://localhost:%d/zone/info", manager->rest_port());
//     auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
//     client.init(opts);
//
//     std::string value = "124";
//
//     auto response = client.post(url).body(value).send();
//         dout(-1) << "Client Info: post request " << url << dendl;
//
//     std::promise<bool> prom;
//     auto fu = prom.get_future();
//     response.then(
//         [&](Http::Response res) {
//         dout(-1) << "Manager Info: " << res.body() << dendl;
//         prom.set_value(true);
//         },
//         Async::IgnoreException);
//     EXPECT_TRUE(fu.get());
//     client.shutdown();
 }

 TEST_F(HVSZoneTest, Share) {
//     Http::Client client;
//     char url[256];
//     snprintf(url, 256, "http://localhost:%d/zone/share", manager->rest_port());
//     auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
//     client.init(opts);
//
//     ZoneShareReq req;
//     req.zoneID = "21219e04-f2a2-4d8b-80db-a254caaf9d7e";
//     req.ownerID = "128";
//     req.memberID.emplace_back("124");
//     req.memberID.emplace_back("128");
//
//     std::string value = req.serialize();
//
//     auto response = client.post(url).body(value).send();
//         dout(-1) << "Client Info: post request " << url << dendl;
//
//     std::promise<bool> prom;
//     auto fu = prom.get_future();
//     response.then(
//         [&](Http::Response res) {
//         dout(-1) << "Manager Info: " << res.body() << dendl;
//         prom.set_value(true);
//         },
//         Async::IgnoreException);
//     EXPECT_TRUE(fu.get());
//     client.shutdown();
 }

 TEST_F(HVSZoneTest, ShareCancel) {
     Http::Client client;
     char url[256];
     snprintf(url, 256, "http://localhost:%d/zone/sharecancel", manager->rest_port());
     auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
     client.init(opts);

     ZoneShareReq req;
     req.zoneID = "21219e04-f2a2-4d8b-80db-a254caaf9d7e";
     req.ownerID = "128";
     req.memberID.emplace_back("123");
//     req.memberID.emplace_back("16");

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

 TEST_F(HVSZoneTest, ZoneRegister) {
//     Http::Client client;
//     char url[256];
//     snprintf(url, 256, "http://localhost:%d/zone/register", manager->rest_port());
//     auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
//     client.init(opts);
//
//     ZoneRegisterReq req;
//     req.zoneName = "yxzone";
//     req.ownerID = "128";
//     req.memberID.emplace_back("123");
//     req.memberID.emplace_back("125");
//     req.spaceName = "changshaspace";
//     req.spaceSize = 100;
//     SpaceMetaData tmpm;
//     tmpm.hostCenterName = "zhongkeyuan1";
//     tmpm.storageSrcName = "center_1";
//     req.spacePathInfo = tmpm.serialize();
//
//     std::string value = req.serialize();
//
//     auto response = client.post(url).body(value).send();
//         dout(-1) << "Client Info: post request " << url << dendl;
//
//     std::promise<bool> prom;
//     auto fu = prom.get_future();
//     response.then(
//         [&](Http::Response res) {
//         dout(-1) << "Manager Info: " << res.body() << dendl;
//         prom.set_value(true);
//         },
//         Async::IgnoreException);
//     EXPECT_TRUE(fu.get());
//     client.shutdown();
 }

 TEST_F(HVSZoneTest, ZoneCancel) {
//     Http::Client client;
//     char url[256];
//     snprintf(url, 256, "http://localhost:%d/zone/cancel", manager->rest_port());
//     auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
//     client.init(opts);
//
//     ZoneCancelReq req;
//     req.zoneID = "7bf24cee-6499-47f1-b9d8-90137255c2a2";
//     req.ownerID = "125";
//
//     std::string value = req.serialize();
//
//     auto response = client.post(url).body(value).send();
//         dout(-1) << "Client Info: post request " << url << dendl;
//
//     std::promise<bool> prom;
//     auto fu = prom.get_future();
//     response.then(
//         [&](Http::Response res) {
//         dout(-1) << "Manager Info: " << res.body() << dendl;
//         prom.set_value(true);
//         },
//         Async::IgnoreException);
//     EXPECT_TRUE(fu.get());
//     client.shutdown();
 }


 TEST_F(HVSZoneTest, MapAdd) {
//   Http::Client client;
//   char url[256];
//   snprintf(url, 256, "http://localhost:%d/zone/mapadd", manager->rest_port());
//   auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
//   client.init(opts);
//
//   MapAddReq req;
//   req.zoneID = "b71a3ad5-513e-4078-9f9b-d9eeb0691b1e";
//   req.ownerID = "123";
//   req.spaceName = "XXXXXXXXXXXXXXXXXXXX";
//   req.spaceSize = 100;
//   SpaceMetaData tmpm;
//   tmpm.hostCenterName = "zhongkeyuan2";
//   tmpm.storageSrcName = "center_2";
//   req.spacePathInfo = tmpm.serialize();
//
//   std::string value = req.serialize();
//
//   auto response = client.post(url).body(value).send();
//         dout(-1) << "Client Info: post request " << url << dendl;
//
//   std::promise<bool> prom;
//   auto fu = prom.get_future();
//   response.then(
//       [&](Http::Response res) {
//         dout(-1) << "Manager Info: " << res.body() << dendl;
//         prom.set_value(true);
//       },
//       Async::IgnoreException);
//   EXPECT_TRUE(fu.get());
//   client.shutdown();
 }


 TEST_F(HVSZoneTest, MapDeduct) {
//   Http::Client client;
//   char url[256];
//   snprintf(url, 256, "http://localhost:%d/zone/mapdeduct", manager->rest_port());
//   auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
//   client.init(opts);
//
//   MapDeductReq req;
//   req.zoneID = "b71a3ad5-513e-4078-9f9b-d9eeb0691b1e";
//   req.ownerID = "123";
//   req.spaceID.emplace_back("8fb17838-f3e1-43ae-8a60-6bc122672013");
//   req.spaceID.emplace_back("873b5bac-1218-4737-82f1-d1b4f027139c");
//
//   std::string value = req.serialize();
//
//   auto response = client.post(url).body(value).send();
//   dout(-1) << "Client Info: post request " << url << dendl;
//
//   std::promise<bool> prom;
//   auto fu = prom.get_future();
//   response.then(
//       [&](Http::Response res) {
//         dout(-1) << "Manager Info: " << res.body() << dendl;
//         prom.set_value(true);
//       },
//       Async::IgnoreException);
//   EXPECT_TRUE(fu.get());
//   client.shutdown();
 }

 TEST_F(HVSZoneTest, ZoneAdd) {
//     Http::Client client;
//     char url[256];
//     snprintf(url, 256, "http://localhost:%d/zone/add", manager->rest_port());
//     auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
//     client.init(opts);
//
//     ZoneRegisterReq req;
//     req.zoneName = "yxzone2";
//     req.ownerID = "128";
//     req.memberID.emplace_back("123");
//     req.memberID.emplace_back("124");
//     SpaceMetaData tmpm;
//     tmpm.hostCenterName = "zhongkeyuan2";
//     tmpm.storageSrcName = "center_2";
//     tmpm.spacePath = "62621dea-bd6b-4c12-b1ce-5b466eea69af";
//     req.spacePathInfo = tmpm.serialize();
//
//     std::string value = req.serialize();
//
//     auto response = client.post(url).body(value).send();
//         dout(-1) << "Client Info: post request " << url << dendl;
//
//     std::promise<bool> prom;
//     auto fu = prom.get_future();
//     response.then(
//         [&](Http::Response res) {
//         dout(-1) << "Manager Info: " << res.body() << dendl;
//         prom.set_value(true);
//         },
//         Async::IgnoreException);
//     EXPECT_TRUE(fu.get());
//     client.shutdown();
 }


// TEST_F(HVSZoneTest, simpleInfo){
//     Zone tmp;
//     std::shared_ptr<hvs::CouchbaseDatastore> zonePtr = std::make_shared<hvs::CouchbaseDatastore>(
//           hvs::CouchbaseDatastore("zone_info"));
//     zonePtr->init();
//     std::string tmp_key = "aeed1d09-779d-4b8b-9005-c4c73d8b5ba1";
//     auto [vp, err] = zonePtr->get(tmp_key);
//     std::string tmp_value = *vp;//待插入报错
//     std::cout << tmp_value << std::endl;

//     Space tmps;
//     std::shared_ptr<hvs::CouchbaseDatastore> spacePtr = std::make_shared<hvs::CouchbaseDatastore>(
//           hvs::CouchbaseDatastore("space_info"));
//     spacePtr->init();
//     tmp_key = "ee9637e0-f13f-46d5-b15d-38dcf1043708";
//     auto [vps, errs] = spacePtr->get(tmp_key);
//     tmp_value = *vps;//待插入报错
//     std::cout << tmp_value << std::endl;

// }


