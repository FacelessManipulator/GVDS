#include <pistache/client.h>
#include <unistd.h>
#include <atomic>
#include <chrono>
#include "context.h"
#include "gtest/gtest.h"
#include "manager/manager.h"
#include "hvs_struct.h"

using namespace std;
using namespace hvs;

class ManagerIOProxyTest : public ::testing::Test {
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
std::string ManagerIOProxyTest::uuid = "";

TEST_F(ManagerIOProxyTest, ioproxy_add) {
  Http::Client client;
  char url[256];
  snprintf(url, 256, "http://localhost:%d/ioproxy", manager->rest_port());
  auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
  client.init(opts);

  // add ioproxy
  IOProxyNode node;
  node.name = "test_ioproxy_node";
  node.ip = "127.0.0.1";
  node.rpc_port = 9091;
  node.data_port = 9092;
  auto response = client.post(url).body(node.serialize()).send();
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

TEST_F(ManagerIOProxyTest, ioproxy_list) {
    Http::Client client;
    char url[256];
    snprintf(url, 256, "http://localhost:%d/ioproxy", manager->rest_port());
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

TEST_F(ManagerIOProxyTest, ioproxy_del) {
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