#include "context.h"
#include "gtest/gtest.h"
#include "msg/rpc.h"
#include "msg/stat.h"
#include <unistd.h>
#include <chrono>
#include <atomic>
#include "io_proxy/io_proxy.h"

using namespace std;
using namespace hvs;

class IOPROXYTEST : public ::testing::Test {
 protected:
  void SetUp() override {
    ioproxy = static_cast<IOProxy*>(HvsContext::get_context()->node);
    ASSERT_NE(ioproxy, nullptr);
  }
  void TearDown() override {
    ioproxy = nullptr;
  }

protected:
    static void SetUpTestCase() {
        hvs::init_context();
        hvs::init_ioproxy();
    }
    static void TearDownTestCase() {
        hvs::destroy_ioproxy(static_cast<IOProxy*>(HvsContext::get_context()->node));
        hvs::destroy_context();
  }
 public:
  IOProxy* ioproxy;
};

TEST_F(IOPROXYTEST, simple) {
    atomic_long cnt = 0;
    long timeout = 3000; // timeout 3s
  auto op = make_shared<IOProxyMetadataOP>();
  op->id = 0;
  op->operation = IOProxyMetadataOP::stat;
  op->path = "/tmp/a";
  op->retval = nullptr;
  op->type = IO_PROXY_METADATA;
  op->complete_callbacks.emplace_back([&](){++cnt;});
  auto start_t = std::chrono::steady_clock::now();
  for(int i = 0; i < 5000; i++) {
      ioproxy->queue_op(op);
  }
  while(cnt < 5000 && timeout >= 0) {
    usleep(100);
    timeout -= 100;
  }
  auto end_t = std::chrono::steady_clock::now();
  dout(-1) << "finish 5000 op in " << std::chrono::duration_cast<std::chrono::milliseconds>(end_t - start_t).count() << "ms" << dendl;
  EXPECT_EQ(cnt, 5000);
}

TEST_F(IOPROXYTEST, SEQ) {
    atomic_long cnt = 0;
    long timeout = 3; // timeout 10s
  auto op = make_shared<IOProxyMetadataOP>();
  op->id = 0;
  op->operation = IOProxyMetadataOP::stat;
  op->path = "/tmp/a";
  op->retval = nullptr;
  op->type = IO_PROXY_METADATA;
  op->complete_callbacks.emplace_back([&](){++cnt;});
  auto start_t = std::chrono::steady_clock::now();
  for(int i = 0; i < 5000; i++) {
      ioproxy->queue_and_wait(make_shared<IOProxyMetadataOP>(*op));
  }
  auto end_t = std::chrono::steady_clock::now();
  dout(-1) << "finish 5000 seq op in " << std::chrono::duration_cast<std::chrono::milliseconds>(end_t - start_t).count() << "ms" << dendl;
  EXPECT_EQ(cnt, 5000);
}