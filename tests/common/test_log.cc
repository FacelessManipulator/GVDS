#include "common/Thread.h"
#include "common/debug.h"
#include "gtest/gtest.h"
#include "include/context.h"
#include "log/Log.h"

using namespace hvs;

class LogTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
  static void SetUpTestCase() { hvs::init_context(); }
  static void TearDownTestCase() { hvs::destroy_context(); }

 public:
};

TEST_F(LogTest, Simple) {
  auto log = hvs::HvsContext::get_context()->_log;
  for (int i = 0; i < 100; i++) {
    hvs::EntryPtr e = log->create_entry(1, "hello world");
    log->submit_entry(e);
  }
  // EXPECT_TRUE("the file in /tmp/hvs.logtest should be correct");
}

TEST_F(LogTest, Dout) {
  for (int i = 0; i < 100; i++) {
    dout(1) << "hello Dout" << dendl;
  }
  // EXPECT_TRUE("the file in /tmp/hvs.logtest should be correct");
}

class LogTester : public Thread {
 public:
  void* entry() override {
    for (int i = 0; i < 10; i++)
      dout(1) << name() << " is doing the log test." << dendl;
  }
};

TEST_F(LogTest, MultiThread) {
  LogTester lts[10];
  char name[16];
  for (int i = 0; i < 10; i++) {
    snprintf(name, 16, "LogTester-%d", i);
    lts[i].create(name);
  }
  for (int i = 0; i < 10; i++) {
    lts[i].join();
  }
  // EXPECT_TRUE("the file in /tmp/hvs.logtest should be correct");
}
