#include "common/Thread.h"
#include "common/debug.h"
#include "gtest/gtest.h"
#include "include/context.h"
#include "log/Log.h"

hvs::Log* start_logger() {
  hvs::Log* log = new hvs::Log;
  log->start();

  log->set_log_file("/tmp/hvs.logtest");
  log->set_log_level(20);
  log->reopen_log_file();
  return log;
}

void stop_log(hvs::Log* log) {
  log->flush();
  log->stop();
  delete log;
}

TEST(LogTest, Simple) {
  auto log = start_logger();
  for (int i = 0; i < 100; i++) {
    hvs::EntryPtr e = log->create_entry(1, "hello world");
    log->submit_entry(e);
  }
  stop_log(log);
  // EXPECT_TRUE("the file in /tmp/hvs.logtest should be correct");
}

TEST(LogTest, Dout) {
  auto log = start_logger();
  auto cct = hvs::HvsContext::get_context();
  cct->_log = log;
  for (int i = 0; i < 100; i++) {
    dout(1) << "hello Dout" << dendl;
  }
  stop_log(log);
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
  auto log = start_logger();
  auto cct = hvs::HvsContext::get_context();
  cct->_log = log;

  LogTester lts[10];
  char name[16];
  for (int i = 0; i < 10; i++) {
    snprintf(name, 16, "LogTester-%d", i);
    lts[i].create(name);
  }
  for (int i = 0; i < 10; i++) {
    lts[i].join();
  }
  stop_log(log);
  // EXPECT_TRUE("the file in /tmp/hvs.logtest should be correct");
}
