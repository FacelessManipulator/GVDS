#include <pistache/client.h>
#include <unistd.h>
#include <atomic>
#include <chrono>
#include "context.h"
#include "gtest/gtest.h"
#include "manager/manager.h"
#include "aggregation_struct.h"

using namespace std;
using namespace hvs;

class ManagerResAggregationTest : public ::testing::Test {
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
  static std::string uuid;
};
std::string ManagerResAggregationTest::uuid = "";

TEST_F(ManagerResAggregationTest, ioproxy_add) {
  Http::Client client;
  char url[256];
  snprintf(url, 256, "http://localhost:%d/resource/register", manager->rest_port());
  auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
  client.init(opts);

  
  StorageResource newRes; 
  newRes.storage_src_id = "1";                 // 存储资源UUID
  newRes.storage_src_name = "center_1";        // 存储资源名称
  newRes.host_center_id = "100";               // 存储资源所在超算中心UUID
  newRes.host_center_name = "zhongkeyuan";     // 存储资源所在超算中心名称
  newRes.total_capacity = 1024;                // 存储资源空间容量大小
  newRes.assign_capacity = 0;                  // 已经分配空间容量大小
  newRes.mgs_address = "http://193.195.34.65"; // 存储资源MGS地址
  newRes.state = Normal;                       // 存储资源状态



  auto response = client.post(url).body(newRes.serialize()).send();
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

TEST_F(ManagerResAggregationTest, ioproxy_list) {
    Http::Client client;
    char url[256];
    snprintf(url, 256, "http://localhost:%d/resource/query", manager->rest_port());
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);

    // list ioproxy
    std::promise<bool> prom;
    auto response = client.get(url).send();
    dout(-1) << "Client Info: get request " << url << dendl;
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

TEST_F(ManagerResAggregationTest, ioproxy_del) {
    Http::Client client;
    char url[256];
    snprintf(url, 256, "http://localhost:%d/ioproxy/%s", manager->rest_port(), uuid.c_str());
    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);

    // del ioproxy
    std::promise<bool> prom;
    auto response = client.del(url).send();
    dout(-1) << "Client Info: del request " << url << dendl;
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