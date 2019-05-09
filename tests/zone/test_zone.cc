#include <iostream>
#include "zone/Zone.h"
#include "zone/ZoneServer.h"
#include "common/JsonSerializer.h"
#include "context.h"
#include "datastore/datastore.h"
#include "common/json.h"
#include "datastore/couchbase_helper.h"
#include "gtest/gtest.h"

#include <atomic>



using namespace hvs;

class HVSZoneTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
  static void SetUpTestCase() { hvs::init_context(); }
  static void TearDownTestCase() { hvs::destroy_context(); }
};
/*
TEST_F(HVSZoneTest, Rename) {
    cout<< "******start client: zonerename ******"<<endl;

    // 第二个参数传地址 第三个参数传请求数量 默认1
    std::string page = "http://localhost:9080/zone/rename";//gai
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

    ZoneRenameReq req;
    req.zoneID = "1213411";
    req.ownerID = "123";
    req.newZoneName = "zonerenametest3";

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

    cout<< "******endl client: zonerename ******"<<endl;
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
}*/
TEST_F(HVSZoneTest, ZoneRegister) {
  Http::Client client;
  char url[256];
  snprintf(url, 256, "http://localhost:9080/zone/register", manager->rest_port());
  auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
  client.init(opts);

  ZoneRegisterReq req;
  req.zoneName = "syzone";
  req.ownerID = "123";
  req.memberID.emplace_back("124");
  req.memberID.emplace_back("125");
  req.spaceName = "beijingspace";
  req.spaceSize = 100;
  SpaceMetaData tmpm;
  tmpm.hostCenterName = "beijing"
  req.spacePathInfo = tmpm.serialize();

  std::string value = req.serialize();

  auto response = client.post(url).body(value).send();
        dout(-1) << "Client Info: post request " << url << dendl;

  std::promise<bool> prom;
  auto fu = prom.get_future();
  response.then(
      [&](Http::Response res) {
        dout(-1) << "Manager Info: " << res.body() << dendl;
        uuid = res.body();
        prom.set_value(true);
      },
      Async::IgnoreException);
  EXPECT_TRUE(fu.get());
  client.shutdown();
}

// TEST_F(HVSZoneTest, ZoneRegister) {
//     cout<< "******start client: zoneregister ******"<<endl;

//     // 第二个参数传地址 第三个参数传请求数量 默认1
//     std::string page = "http://localhost:9080/zone/register";//gai
//     int count = 1;

//     Http::Client client;

//     auto opts = Http::Client::options()
//         .threads(1)
//         .maxConnectionsPerHost(8);
//     client.init(opts);

//     std::vector<Async::Promise<Http::Response>> responses;

//     std::atomic<size_t> completedRequests(0);
//     std::atomic<size_t> failedRequests(0);

//     //计时
//     auto start = std::chrono::steady_clock::now();

//     ZoneRegisterReq req;
//     req.zoneName = "syzone";
//     req.ownerID = "123";
//     req.memberID.emplace_back("124");
//     req.memberID.emplace_back("125");
//     req.spaceName = "beijingspace";
//     req.spaceSize = 100;
//     SpaceMetaData tmpm;
//     tmpm.hostCenterName = "beijing"
//     req.spacePathInfo = tmpm.serialize();

//     std::string value = req.serialize();

//     auto resp = client.post(page).cookie(Http::Cookie("FOO", "bar")).body(value).send();
//     resp.then([&](Http::Response response) {
//             ++completedRequests;
//         std::cout << "Response code = " << response.code() << std::endl;
//         //response body
//         auto body = response.body();
//         if (!body.empty()){
//             std::cout << "Response body = " << body << std::endl;
//             //====================
//             //your code write here

//             //====================
//         }
//     }, Async::IgnoreException);
//     responses.push_back(std::move(resp));

//     auto sync = Async::whenAll(responses.begin(), responses.end());
//     Async::Barrier<std::vector<Http::Response>> barrier(sync);
//     barrier.wait_for(std::chrono::seconds(5));

//     auto end = std::chrono::steady_clock::now();
//     std::cout << "Summary of execution" << std::endl
//               << "Total number of requests sent     : " << count << std::endl
//               << "Total number of responses received: " << completedRequests.load() << std::endl
//               << "Total number of requests failed   : " << failedRequests.load() << std::endl
//               << "Total time of execution           : "
//               << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

//     client.shutdown();

//     cout<< "******endl client: zonecancel ******"<<endl;
// }

/*
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
}*/



// TEST_F(HVSZoneTest, 123)
// {
//     Space tmps;
//     std::shared_ptr<hvs::CouchbaseDatastore> spacePtr = std::make_shared<hvs::CouchbaseDatastore>(
//           hvs::CouchbaseDatastore("space_info"));
//     spacePtr->init();
//     auto [vs, er] = spacePtr->get("1213411-1");
//     std::cout << *vs << std::endl;
// }
/*
TEST_F(HVSZoneTest, Simple) {
  Zone z1;
  Zone z2;
  z1.zoneID = "1213411";
  //z1.zoneID=uuid//字符串形式uuid，在生成zone的时候产生
  z1.zoneName = "zone1";
  z1.ownerID = "123";
  z1.memberID.emplace_back("2");
  z1.memberID.emplace_back("3");
  z1.spaceID.emplace_back("space1");//字符串形式uuid，在生成space的时候产生
  z1.spaceID.emplace_back("space2");

  std::string z1_json = z1.serialize();
  std::cout << z1_json << std::endl;
  z2.deserialize(z1_json);
  std::cout << z2.serialize() << std::endl;

  // EXPECT_TRUE("the file in /tmp/hvs.logtest should be correct");
}

TEST_F(HVSZoneTest, Datastore) {
  Zone z1;
  z1.zoneID = "1213411";
  //z1.zoneID=uuid//字符串形式uuid，在生成zone的时候产生
  z1.zoneName = "zone1";
  z1.ownerID = "123";
  z1.memberID.emplace_back("2");
  z1.memberID.emplace_back("3");
  z1.spaceID.emplace_back("1213411-1");//字符串形式uuid，在生成space的时候产生
  z1.spaceID.emplace_back("1213411-2");
  std::string z1_value = z1.serialize();
  std::string z1_key = z1.zoneID;
  std::shared_ptr<hvs::CouchbaseDatastore> dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("zone_info"));
  dbPtr->init();
  EXPECT_EQ(0, dbPtr->set(z1_key, z1_value));
  auto[vp, err] = dbPtr->get(z1_key);
  EXPECT_EQ(z1_value, *(vp));
  //EXPECT_EQ(z1_value, *(dbPtr->get(z1_key)));//实验老方法get是否可行
  //    EXPECT_EQ(0, dbPtr->remove(z1_key));
  //    EXPECT_EQ("", *(dbPtr->get(z1_key)));
}*/


/*



TEST_F(HVSZoneTest, GetInfo){
  //记录一个新的zone
  Zone z1;
  z1.zoneID = "1213412";
  //z1.zoneID=uuid//字符串形式uuid，在生成zone的时候产生
  z1.zoneName = "zone2";
  z1.ownerID = "124";
  z1.memberID.emplace_back("2");
  z1.memberID.emplace_back("123");
  z1.spaceID.emplace_back("1213411-1");//字符串形式uuid，在生成space的时候产生
  z1.spaceID.emplace_back("1213411-2");
  std::string z1_value = z1.serialize();
  std::string z1_key = z1.zoneID;
  std::shared_ptr<hvs::CouchbaseDatastore> dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("zone_info"));
  dbPtr->init();
  EXPECT_EQ(0, dbPtr->set(z1_key, z1_value));
  std::vector<std::string> test_result;
  if(GetZoneInfo(test_result, "153"))
  {
    for (std::vector<std::string>::iterator m = test_result.begin(); m != test_result.end(); m++)
    {
      std::cout << *m << std::endl;
      //std::cout << (*m).serialize() << std::endl;
      //std::cout << (*m).zoneID << std::endl;
    }
  }
  else 
  std::cout << "no such zone" << std::endl;
}

TEST_F(HVSZoneTest, share){
  std::vector<std::string> mem;
  mem.emplace_back("15");
  mem.emplace_back("30");
  EXPECT_EQ(0, ZoneShare("1213412", "124", mem));
  //EXPECT_EQ(0, ZoneShareCancel("1213412", "124", mem));
}

*/
