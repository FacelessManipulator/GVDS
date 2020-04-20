#include "common/Thread.h"
#include "gvds_context.h"
#include "gtest/gtest.h"
#include "log/Log.h"

using namespace gvds;

class LogTest : public ::testing::Test {
protected:
    void SetUp() override {
        init_context();
        log = HvsContext::get_context()->_log;
    }
    void TearDown() override {
        destroy_context();
        log = nullptr;
    }

public:
    Log* log;
};

TEST_F(LogTest, Simple) {
  for (int i = 0; i < 10; i++) {
    gvds::EntryPtr e = log->create_entry(1, "hello world");
    log->submit_entry(e);
  }
  // EXPECT_TRUE("the file in /tmp/gvds.logtest should be correct");
}

TEST_F(LogTest, Dout) {
  for (int i = 0; i < 10; i++) {
    dout(1) << "hello Dout" << dendl;
  }
  // EXPECT_TRUE("the file in /tmp/gvds.logtest should be correct");
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
  // EXPECT_TRUE("the file in /tmp/gvds.logtest should be correct");
}
