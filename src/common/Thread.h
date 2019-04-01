#pragma once

/***********************************************************
 * @file  Thread.h
 * @brief 线程类文件，提供了线程运行函数以及抽象继承接口
 * @author 周汉杰
 * @version 0.0.1
 * @date 2018/3/1
 * @email miller_maxwell@buaa.edu.cn
 * @license GNU General Public License (GPL)
 *
 * 修改历史：
 * ----------------------------------------------
 * 日期     | 版本号  |   作者   |      描述
 * ----------------------------------------------
 * 2018/3/1 | 0.0.1  | 周汉杰   | 实现基础功能
 * ----------------------------------------------
 *
 *
 ***********************************************************/

#include <system_error>
#include <thread>

#include <pthread.h>
#include <sys/types.h>
#include <string>

/// 线程类，提供线程工具函数
/**
 * @author: 周汉杰
 * @date: 2018/3/1
 *
 * 线程类基于linux线程库libpthread进行开发，引用后编译时需要 -lpthread。
 */
class Thread {
 private:
  pthread_t thread_id;
  pid_t pid;
  int cpuid;
  std::string thread_name;

  void *entry_wrapper();

 public:
  Thread(const Thread &) = delete;
  Thread &operator=(const Thread &) = delete;

  Thread();
  virtual ~Thread();

 protected:
  virtual void *entry() = 0;

 private:
  static void *_entry_func(void *arg);

 public:
  const pthread_t &get_thread_id() const;
  const char *name() { return thread_name.c_str(); }
  pid_t get_pid() const { return pid; }
  bool is_started() const;
  bool am_self() const;
  int kill(int signal);
  void create(const char *name);
  int join(void **prval = 0);
  int detach();
  int set_affinity(int cpuid);
};