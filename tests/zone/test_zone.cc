#include <iostream>
#include "gvds_struct.h"
#include "manager/zone/ZoneServer.h"
#include "common/JsonSerializer.h"
#include "gvds_context.h"
#include "datastore/datastore.h"
#include "common/json.h"
#include "datastore/couchbase_helper.h"
#include "gtest/gtest.h"
#include <pistache/client.h>
#include <atomic>
#include <future>



using namespace gvds;

class GVDSZoneTest : public ::testing::Test {
 protected:
  void SetUp() override {
//    manager = static_cast<Manager*>(HvsContext::get_context()->node);
//    ASSERT_NE(manager, nullptr);
  }
  void TearDown() override {
//      manager = nullptr;
  }

 protected:
  static void SetUpTestCase() {
    gvds::init_context();
//    gvds::init_manager();
    usleep(100000); // wait 100 ms. rest server may started.
  }
  static void TearDownTestCase() {
//    gvds::destroy_manager(static_cast<Manager*>(HvsContext::get_context()->node));
    gvds::destroy_context();
  }

 public:
//  Manager* manager;
};

TEST_F(GVDSZoneTest, Rename) {
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

 TEST_F(GVDSZoneTest, Locate) {
    //  Http::Client client;
    //  char url[256];
    //  snprintf(url, 256, "http://localhost:%d/zone/locate", 37961);
    //  auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    //  client.init(opts);

    //  GetZoneLocateInfoReq req;
    //  req.clientID = "128";
    //  req.zoneID = "21219e04-f2a2-4d8b-80db-a254caaf9d7e";
    //  req.spaceID.emplace_back("62621dea-bd6b-4c12-b1ce-5b466eea69af");

    //  std::string value = req.serialize();

    //  auto response = client.post(url).body(value).send();
    //      dout(-1) << "Client Info: post request " << url << dendl;

    //  std::promise<bool> prom;
    //  auto fu = prom.get_future();
    //  response.then(
    //      [&](Http::Response res) {
    //      dout(-1) << "Manager Info: " << res.body() << dendl;
    //      prom.set_value(true);
    //      },
    //      Async::IgnoreException);
    //  EXPECT_TRUE(fu.get());
    //  client.shutdown();
 }

 TEST_F(GVDSZoneTest, GetInfo) {
    //  Http::Client client;
    //  char url[256];
    //  snprintf(url, 256, "http://localhost:%d/zone/info", 37909);
    //  auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    //  client.init(opts);

    //  std::string value = "124";

    //  auto response = client.post(url).body(value).send();
    //      dout(-1) << "Client Info: post request " << url << dendl;

    //  std::promise<bool> prom;
    //  auto fu = prom.get_future();
    //  response.then(
    //      [&](Http::Response res) {
    //      dout(-1) << "Manager Info: " << res.body() << dendl;
    //      prom.set_value(true);
    //      },
    //      Async::IgnoreException);
    //  EXPECT_TRUE(fu.get());
    //  client.shutdown();
 }

 TEST_F(GVDSZoneTest, Share) {
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

 TEST_F(GVDSZoneTest, ShareCancel) {
//     Http::Client client;
//     char url[256];
//     snprintf(url, 256, "http://localhost:%d/zone/sharecancel", manager->rest_port());
//     auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
//     client.init(opts);
//
//     ZoneShareReq req;
//     req.zoneID = "21219e04-f2a2-4d8b-80db-a254caaf9d7e";
//     req.ownerID = "128";
//     req.memberID.emplace_back("123");
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

 TEST_F(GVDSZoneTest, ZoneRegister) {
//      Http::Client client;
//      char url[256];
//      snprintf(url, 256, "http://192.168.10.219:%d/zone/register", 34299);
// //     snprintf(url, 256, "http://localhost:%d/zone/register", 55877);
//      auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
//      client.init(opts);

//      //本地区域创建 101 102 103
// //     ZoneRegisterReq req;
// //     req.zoneName = "bqzone";
// //     req.ownerID = "103";
// //     req.memberID.emplace_back("102");
// //     //req.memberID.emplace_back("103");
// //     req.spaceName = "bingqlocalspace";
// //     req.spaceSize = 200;
// //     SpaceMetaData tmpm;
// //     tmpm.hostCenterName = "beihang";
// //     tmpm.storageSrcName = "localstorage";
// //     req.spacePathInfo = tmpm.serialize();

//         //远程区域创建 201 202
//         ZoneRegisterReq req;
//         req.zoneName = "compute-zone";
//         req.ownerID = "000";
// //        req.memberID.emplace_back("202");
//         //req.memberID.emplace_back("103");
//         req.spaceName = "localcompute";
//         req.spaceSize = 200;
//         SpaceMetaData tmpm;
//         tmpm.hostCenterName = "zhongkeyuan";
// //        tmpm.storageSrcName = "lustre";
//         req.spacePathInfo = tmpm.serialize();

//      std::string value = req.serialize();

//      auto response = client.post(url).body(value).send();
//          dout(-1) << "Client Info: post request " << url << dendl;

//      std::promise<bool> prom;
//      auto fu = prom.get_future();
//      response.then(
//          [&](Http::Response res) {
//          dout(-1) << "Manager Info: " << res.body() << dendl;
//          prom.set_value(true);
//          },
//          Async::IgnoreException);
//      EXPECT_TRUE(fu.get());
//      client.shutdown();
 }

 TEST_F(GVDSZoneTest, ZoneCancel) {
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


 TEST_F(GVDSZoneTest, MapAdd) {
  //  Http::Client client;
  //  char url[256];
  //  snprintf(url, 256, "http://127.0.0.1:%d/zone/mapadd", 53953);
  //  auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
  //  client.init(opts);

  //  MapAddReq req;
  //  req.zoneID = "dca23148-b195-4ea0-b542-b3d08f1667d7";
  //  req.ownerID = "202";
  //  req.spaceName = "SYBUAA新空间";
  //  req.spaceSize = 100;
  //  SpaceMetaData tmpm;
  //  tmpm.hostCenterName = "beihang";
  //  tmpm.storageSrcName = "localstorage";
  //  req.spacePathInfo = tmpm.serialize();

  //  std::string value = req.serialize();

  //  auto response = client.post(url).body(value).send();
  //        dout(-1) << "Client Info: post request " << url << dendl;

  //  std::promise<bool> prom;
  //  auto fu = prom.get_future();
  //  response.then(
  //      [&](Http::Response res) {
  //        dout(-1) << "Manager Info: " << res.body() << dendl;
  //        prom.set_value(true);
  //      },
  //      Async::IgnoreException);
  //  EXPECT_TRUE(fu.get());
  //  client.shutdown();
 }


 TEST_F(GVDSZoneTest, MapDeduct) {
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

 TEST_F(GVDSZoneTest, ZoneAdd) {
    // Http::Client client;
    // char url[256];
    // snprintf(url, 256, "http://localhost:%d/zone/add", 51521);
    // auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    // client.init(opts);

    // ZoneRequest req;
    // req.zoneName = "compute-zone";
    // req.ownerID = "000";
    // req.memberID.emplace_back("111");
    // req.memberID.emplace_back("222");
    // Space tmpm;
    // tmpm.hostCenterName = "beihang";
    // tmpm.storageSrcName = "localstorage";
    // tmpm.spacePath = "6f607624-d570-4cd0-afcc-574bec802110";
    // req.spacePathInfo = tmpm.serialize();

    // std::string value = req.serialize();

    // auto response = client.post(url).body(value).send();
    //     dout(-1) << "Client Info: post request " << url << dendl;

    // std::promise<bool> prom;
    // auto fu = prom.get_future();
    // response.then(
    //     [&](Http::Response res) {
    //     dout(-1) << "Manager Info: " << res.body() << dendl;
    //     prom.set_value(true);
    //     },
    //     Async::IgnoreException);
    // EXPECT_TRUE(fu.get());
    // client.shutdown();
 }


TEST_F(GVDSZoneTest, simpleInfo){
    Zone tmp;
    std::shared_ptr<gvds::CouchbaseDatastore> zonePtr = std::make_shared<gvds::CouchbaseDatastore>(
          gvds::CouchbaseDatastore("zone_info"));
    zonePtr->init();
    std::string tmp_key = "0";
    tmp.zoneID = "0";
    tmp.zoneName = "hjtest";
    tmp.ownerID = "0";
    tmp.spaceID.push_back("0");
    tmp.spaceID.push_back("1");
    std::string tmp_value = tmp.serialize();//待插入报错   
    zonePtr->set(tmp_key, tmp_value);
    std::cout << tmp_value << std::endl;

    Space tmps;
    std::shared_ptr<gvds::CouchbaseDatastore> spacePtr = std::make_shared<gvds::CouchbaseDatastore>(
          gvds::CouchbaseDatastore("space_info"));
    spacePtr->init();
    tmp_key = "0";
    tmps.spaceID = "0";
    tmps.spaceName = "test1";
    tmps.spaceSize = 10;
    tmps.hostCenterID = "0";
    tmps.storageSrcID = "0";
    tmps.spacePath = "test1";
    tmps.hostCenterName = "h0";
    tmps.storageSrcName = "s0";
    tmps.status = true;
    
    tmp_value = tmps.serialize();//待插入报错   ;//待插入报错
    spacePtr->set(tmp_key, tmp_value);
    std::cout << tmp_value << std::endl;

}


