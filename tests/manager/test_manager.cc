#include <pistache/client.h>
#include <unistd.h>
#include <atomic>
#include <chrono>
#include "gvds_context.h"
#include "gtest/gtest.h"
#include "manager/manager.h"
#include <future>

using namespace std;
using namespace gvds;

class ManagerTest : public ::testing::Test {
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

TEST_F(ManagerTest, simple) {
  Http::Client client;
  char url[256];
  snprintf(url, 256, "http://localhost:%d/manager", manager->rest_port());
  auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
  client.init(opts);
  auto response = client.get(url).send();
        dout(-1) << "Client Info: get request " << url << dendl;

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