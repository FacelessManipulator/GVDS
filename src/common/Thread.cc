#include "common/Thread.h"

#include <assert.h>
#include <sched.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

static int _set_affinity(int id) {
  if (id >= 0 && id < CPU_SETSIZE) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);

    CPU_SET(id, &cpuset);

    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) < 0) return -errno;
    /* guaranteed to take effect immediately */
    sched_yield();
  }
  return 0;
}

Thread::Thread() : thread_id(0), pid(0), cpuid(-1) {}

Thread::~Thread() {}

void *Thread::_entry_func(void *arg) {
  void *r = ((Thread *)arg)->entry_wrapper();
  return r;
}

void *Thread::entry_wrapper() {
  pid_t p = (pid_t)syscall(SYS_gettid);
  if (p > 0) pid = p;
  if (pid && cpuid >= 0) _set_affinity(cpuid);

  pthread_setname_np(pthread_self(), thread_name.c_str());
  return entry();
}

const pthread_t &Thread::get_thread_id() const { return thread_id; }

bool Thread::is_started() const { return thread_id != 0; }

bool Thread::am_self() const { return (pthread_self() == thread_id); }

int Thread::kill(int signal) {
  if (thread_id)
    return pthread_kill(thread_id, signal);
  else
    return -EINVAL;
}

void Thread::create(const char *name) {
  // assert(strlen(name) < 16); // thread name should not longer than 16 char
  thread_name = name;

  int ret = pthread_create(&thread_id, NULL, _entry_func, (void *)this);
  if (ret != 0) {
    char buf[256];
    snprintf(buf, sizeof(buf),
             "Thread::try_create(): pthread_create "
             "failed with error %d",
             ret);
    std::cerr << buf;
    std::cerr.flush();
    assert(ret == 0);
  }
}

int Thread::join(void **prval) {
  if (thread_id == 0) {
    assert("join on thread that was never started" == 0);
    return -EINVAL;
  }

  int status = pthread_join(thread_id, prval);
  if (status != 0) {
    char buf[256];
    snprintf(buf, sizeof(buf),
             "Thread::join(): pthread_join "
             "failed with error %d\n",
             status);
    // Output to stderr, in case that Log might not been initialized yet, or has
    // been destroyed.
    std::cerr << buf;
    std::cerr.flush();
    assert(status == 0);
  }

  thread_id = 0;
  return status;
}

int Thread::detach() { return pthread_detach(thread_id); }

int Thread::set_affinity(int id) {
  int r = 0;
  cpuid = id;
  if (pid && syscall(SYS_gettid) == pid) r = _set_affinity(id);
  return r;
}