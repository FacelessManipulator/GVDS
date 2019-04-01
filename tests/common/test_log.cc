#include "common/Thread.h"
#include "common/debug.h"
#include "gtest/gtest.h"
#include "include/context.h"
#include "log/Log.h"

using namespace hvs;
TEST(LogTest, Simple) {
  auto log = init_logger();
  for (int i = 0; i < 100; i++) {
    hvs::EntryPtr e = log->create_entry(1, "hello world");
    log->submit_entry(e);
  }
  stop_log(log);
  // EXPECT_TRUE("the file in /tmp/hvs.logtest should be correct");
}

TEST(LogTest, Dout) {
  init_context();
  for (int i = 0; i < 100; i++) {
    dout(1) << "hello Dout" << dendl;
  }
  destroy_context();
  // EXPECT_TRUE("the file in /tmp/hvs.logtest should be correct");
}

class LogTester : public Thread {
 public:
  void* entry() override {
    for (int i = 0; i < 10; i++)
      dout(1) << name() << " is doing the log test." << dendl;
  }
};

TEST(LogTest, MultiThread) {
  init_context();
  LogTester lts[10];
  char name[16];
  for (int i = 0; i < 10; i++) {
    snprintf(name, 16, "LogTester-%d", i);
    lts[i].create(name);
  }
  for (int i = 0; i < 10; i++) {
    lts[i].join();
  }
  destroy_context();
  // EXPECT_TRUE("the file in /tmp/hvs.logtest should be correct");
}
