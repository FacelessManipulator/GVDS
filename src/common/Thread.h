#ifndef HVS_THREAD_H
#define HVS_THREAD_H

#include <system_error>
#include <thread>

#include <pthread.h>
#include <sys/types.h>
#include <string>

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
#endif