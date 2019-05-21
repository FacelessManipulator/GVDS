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
/*
TEST_F(HVSZoneTest, Rename) {
  Http::Client client;
  char url[256];
  snprintf(url, 256, "http://localhost:%d/zone/rename", manager->rest_port());
  auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
  client.init(opts);

  ZoneRenameReq req;
  req.zoneID = "1213411";
  req.ownerID = "123";
  req.newZoneName = "zonerenametest3";

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

TEST_F(HVSZoneTest, Locate) {
    cout<< "******start client: zonelocate ******"<<endl;

    // 第二个参数传地址 第三个参数传请求数量 默认1
    std::string page = "http://localhost:9080/zone/locate";//gai
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

    GetZoneLocateInfoReq req;
    req.clientID = "2";
    req.zoneID = "1213411";
    req.spaceID.emplace_back("1213411-1");
    req.spaceID.emplace_back("1213411-2");

    std::string value = req.serialize();

    auto resp = client.post(page).cookie(Http::Cookie("FOO", "bar")).body(value).send();
    resp.then([&](Http::Response response) {
            ++completedRequests;
        std::cout << "Response code = " << response.code() << std::endl;
        //response body
        auto body = response.body();
        if (!body.empty()){
            //GetZoneLocateInfoRes res;
            //res.deserialize(body);
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

    cout<< "******endl client: zonelocate ******"<<endl;
}

TEST_F(HVSZoneTest, GetInfo) {
    cout<< "******start client: getzoneinfo ******"<<endl;

    // 第二个参数传地址 第三个参数传请求数量 默认1
    std::string page = "http://localhost:9080/zone/info";//gai
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

    std::string value = "123";

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

    cout<< "******endl client: getzoneinfo ******"<<endl;
}

TEST_F(HVSZoneTest, Share) {
    cout<< "******start client: zoneshare ******"<<endl;

    // 第二个参数传地址 第三个参数传请求数量 默认1
    std::string page = "http://localhost:9080/zone/share";//gai
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

    ZoneShareReq req;
    req.zoneID = "1213411";
    req.ownerID = "123";
    req.memberID.emplace_back("14");
    req.memberID.emplace_back("16");

    std::string value = req.serialize();

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

    cout<< "******endl client: zoneshare ******"<<endl;
}

TEST_F(HVSZoneTest, ShareCancel) {
    cout<< "******start client: zonesharecancel ******"<<endl;

    // 第二个参数传地址 第三个参数传请求数量 默认1
    std::string page = "http://localhost:9080/zone/sharecancel";//gai
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

    ZoneShareReq req;
    req.zoneID = "1213411";
    req.ownerID = "123";
    req.memberID.emplace_back("14");
    req.memberID.emplace_back("16");

    std::string value = req.serialize();

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

    cout<< "******endl client: zonesharecancel ******"<<endl;
}

TEST_F(HVSZoneTest, ZoneRegister) {
  Http::Client client;
  char url[256];
  snprintf(url, 256, "http://localhost:%d/zone/register", manager->rest_port());
  auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
  client.init(opts);

  ZoneRegisterReq req;
  req.zoneName = "lbqzone";
  req.ownerID = "124";
  req.memberID.emplace_back("123");
  req.memberID.emplace_back("125");
  req.spaceName = "changshaspace";
  req.spaceSize = 100;
  SpaceMetaData tmpm;
  tmpm.hostCenterName = "changsha";
  tmpm.storageSrcName = "lustre1";
  req.spacePathInfo = tmpm.serialize();

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


TEST_F(HVSZoneTest, ZoneCancel) {
    cout<< "******start client: zonecancel ******"<<endl;

    // 第二个参数传地址 第三个参数传请求数量 默认1
    std::string page = "http://localhost:9080/zone/cancel";//gai
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

    ZoneCancelReq req;
    req.zoneID = "1213411";
    req.ownerID = "123";


    std::string value = req.serialize();

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

    cout<< "******endl client: zonecancel ******"<<endl;
}


TEST_F(HVSZoneTest, MapAdd) {
  Http::Client client;
  char url[256];
  snprintf(url, 256, "http://localhost:%d/zone/mapadd", manager->rest_port());
  auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
  client.init(opts);

  MapAddReq req;
  req.zoneID = "aeed1d09-779d-4b8b-9005-c4c73d8b5ba1";
  req.ownerID = "124";
  req.spaceName = "beijingspace";
  req.spaceSize = 58;
  SpaceMetaData tmpm;
  tmpm.hostCenterName = "beijing";
  tmpm.storageSrcName = "lustre1";
  req.spacePathInfo = tmpm.serialize();

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
*/

TEST_F(HVSZoneTest, MapDeduct) {
  Http::Client client;
  char url[256];
  snprintf(url, 256, "http://localhost:%d/zone/mapdeduct", manager->rest_port());
  auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
  client.init(opts);

  MapDeductReq req;
  req.zoneID = "aeed1d09-779d-4b8b-9005-c4c73d8b5ba1";
  req.ownerID = "124";
  req.spaceID.emplace_back("3625ea4b-6759-44a7-b785-ae34d63b2566");
  req.spaceID.emplace_back("ee9637e0-f13f-46d5-b15d-38dcf1043708");

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

// TEST_F(HVSZoneTest, ZoneAdd) {
//   Http::Client client;
//   char url[256];
//   snprintf(url, 256, "http://localhost:%d/zone/add", manager->rest_port());
//   auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
//   client.init(opts);

//   ZoneRegisterReq req;
//   req.zoneName = "wbzone";
//   req.ownerID = "125";
//   req.memberID.emplace_back("123");
//   req.memberID.emplace_back("121");
//   req.spaceName = "changshaspace";
//   req.spaceSize = 100;
//   SpaceMetaData tmpm;
//   tmpm.hostCenterName = "changsha";
//   tmpm.storageSrcName = "lustre1";
//   tmpm.spacePath = "../123/changshaspace";
//   req.spacePathInfo = tmpm.serialize();

//   std::string value = req.serialize();

//   auto response = client.post(url).body(value).send();
//         dout(-1) << "Client Info: post request " << url << dendl;

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
// }


// TEST_F(HVSZoneTest, Info){
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


