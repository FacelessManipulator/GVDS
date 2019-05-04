#include <unistd.h>
#include <atomic>
#include <chrono>
#include "context.h"
#include "gtest/gtest.h"
#include "io_proxy/io_proxy.h"
#include "msg/rpc.h"
#include "msg/stat_demo.h"

#define  TFILEP "/tmp/hvs/tests/data/syncio.txt"

using namespace std;
using namespace hvs;

class IOPROXYTEST : public ::testing::Test {
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

TEST_F(IOPROXYTEST, MetadataOP) {
  std::weak_ptr<IOProxyMetadataOP> op_observe;
  {
    auto op = make_shared<IOProxyMetadataOP>();
    op->id = 0;
    op->operation = IOProxyMetadataOP::stat;
    op->path = TFILEP;
    op->type = IO_PROXY_METADATA;
    op_observe = op;
    auto start_t = std::chrono::steady_clock::now();
    ioproxy->queue_and_wait(op);
    auto end_t = std::chrono::steady_clock::now();
    dout(-1) << "finish metadata op in "
             << std::chrono::duration_cast<std::chrono::milliseconds>(end_t -
                                                                      start_t)
                    .count()
             << "ms. file inode: " << op->buf->st_ino << dendl;
  }
  // the buffer in op should be expired
  usleep(100); // give some time waiting the reset of state machine
  EXPECT_EQ(op_observe.expired(), true);
}

TEST_F(IOPROXYTEST, MULTI) {
  atomic_long cnt = 0;
  long timeout = 3000000;  // timeout 3s
  auto start_t = std::chrono::steady_clock::now();
  for (int i = 0; i < 5000; i++) {
    auto op = make_shared<IOProxyMetadataOP>();
    op->id = i;
    op->operation = IOProxyMetadataOP::stat;
    op->path = TFILEP;
    op->type = IO_PROXY_METADATA;
    op->complete_callbacks.emplace_back([&]() { ++cnt; });
    ioproxy->queue_op(op);
  }
  while (cnt < 5000 && timeout >= 0) {
    usleep(1000);
    timeout -= 1000;
  }
  auto end_t = std::chrono::steady_clock::now();
  dout(-1) << "finish 5000 op in "
           << std::chrono::duration_cast<std::chrono::milliseconds>(end_t -
                                                                    start_t)
                  .count()
           << "ms." << dendl;
  EXPECT_EQ(cnt, 5000);
}

TEST_F(IOPROXYTEST, SEQ) {
  atomic_long cnt = 0;
  long timeout = 3;  // timeout 10s
  auto start_t = std::chrono::steady_clock::now();
  for (int i = 0; i < 5000; i++) {
    auto op = make_shared<IOProxyMetadataOP>();
    op->id = i;
    op->operation = IOProxyMetadataOP::stat;
    op->path = TFILEP;
    op->type = IO_PROXY_METADATA;
    op->complete_callbacks.emplace_back([&]() { ++cnt; });
    ioproxy->queue_and_wait(op);
  }
  auto end_t = std::chrono::steady_clock::now();
  dout(-1) << "finish 5000 seq op in "
           << std::chrono::duration_cast<std::chrono::milliseconds>(end_t -
                                                                    start_t)
                  .count()
           << "ms." << dendl;
  EXPECT_EQ(cnt, 5000);
}

TEST_F(IOPROXYTEST, WRITE) {
  atomic_long cnt = 0;
  long timeout = 3000000;  // timeout 3s
  long times = 10000;
  std::string data("hello!");
  auto start_t = std::chrono::steady_clock::now();
  for (int i = 0; i < times; i++) {
    auto op = make_shared<IOProxyDataOP>();
    op->id = i;
    op->operation = IOProxyDataOP::write;
    op->path = TFILEP;
    op->type = IO_PROXY_DATA;
    op->ibuf = data.c_str();
    op->size = data.size();
    op->complete_callbacks.emplace_back([&]() { ++cnt; });
    ioproxy->queue_op(op);
  }
  while (cnt < times && timeout >= 0) {
    usleep(1000);
    timeout -= 1000;
  }
  auto end_t = std::chrono::steady_clock::now();
  dout(-1) << "finish " << times << " write op in "
           << std::chrono::duration_cast<std::chrono::milliseconds>(end_t -
                                                                    start_t)
                  .count()
           << "ms." << dendl;
  EXPECT_EQ(cnt, times);
}

TEST_F(IOPROXYTEST, READ) {
  atomic_long cnt = 0;
  long timeout = 3000000;  // timeout 3s
  long times = 10000;
  std::string data("hello!");
  auto start_t = std::chrono::steady_clock::now();
  for (int i = 0; i < times; i++) {
    auto op = make_shared<IOProxyDataOP>();
    op->id = i;
    op->operation = IOProxyDataOP::read;
    op->path = TFILEP;
    op->type = IO_PROXY_DATA;
    op->should_prepare = true;
    op->size = data.size();
    op->complete_callbacks.emplace_back([&]() { ++cnt; });
    ioproxy->queue_op(op);
    // EXPECT_STREQ(data.c_str(), op->obuf);
  }
  while (cnt < times && timeout >= 0) {
    usleep(1000);
    timeout -= 1000;
  }
  auto end_t = std::chrono::steady_clock::now();
  dout(-1) << "finish " << times << " seq read op in "
           << std::chrono::duration_cast<std::chrono::milliseconds>(end_t -
                                                                    start_t)
                  .count()
           << "ms." << dendl;
  EXPECT_EQ(cnt, times);
}