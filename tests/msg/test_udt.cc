#include "context.h"
#include "gtest/gtest.h"
#include "msg/rpc.h"
#include <unistd.h>
#include <chrono>
#include "config/ConfigureSettings.h"
#include "io_proxy/rpc_types.h"
#include "io_proxy/rpc_bindings.hpp"
#include "msg/pack.h"
#include "msg/udt_client.h"
#include <vector>
#include <dirent.h>
using namespace std;
using namespace hvs;
#define  TFILEP "/syncio.txt"

class IOProxyUDT : public ::testing::Test {
 protected:
  void SetUp() override {
    ioproxy = static_cast<IOProxy*>(HvsContext::get_context()->node);
    ASSERT_NE(ioproxy, nullptr);
  }
  void TearDown() override { ioproxy = nullptr; }

 protected:
  static void SetUpTestCase() {
    hvs::init_context();
    hvs::init_ioproxy();
  }
  static void TearDownTestCase() {
    hvs::destroy_ioproxy(
        static_cast<IOProxy*>(HvsContext::get_context()->node));
    hvs::destroy_context();
  }

 public:
  IOProxy* ioproxy;
};


TEST_F(IOProxyUDT, simple) {
    UDTClient client;
    auto session = client.create_session("127.0.0.1", 9095);
    ASSERT_TRUE(session);

    string pathname(TFILEP);
    char* buf = new char[7]; //100KB
    memcpy(buf, "hello\n", 6);
    ioproxy_rpc_buffer _buffer(pathname.c_str(), buf, 0, 7);
    int id = session->write(_buffer);
    session->wait_op(id);
}