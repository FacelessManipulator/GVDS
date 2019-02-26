
#ifndef HVS_LOG_LOG_H_
#define HVS_LOG_LOG_H_

#include <chrono>
#include <fstream>
#include <memory>
#include "common/Thread.h"

#include <queue>

namespace hvs {
class Entry;
using EntryPtr = std::shared_ptr<Entry>;
class Log : private Thread {
  using log_lock = std::chrono::system_clock;
  log_lock clock;

  pthread_mutex_t m_queue_mutex;
  pthread_mutex_t m_flush_mutex;
  pthread_cond_t m_cond_loggers;
  pthread_cond_t m_cond_flusher;

  pthread_t m_queue_mutex_holder;
  pthread_t m_flush_mutex_holder;

  std::queue<EntryPtr> m_new;  ///< new entries

  std::string m_log_file;
  std::ofstream m_fstream;

  int m_syslog_log;
  int m_stderr_log;

  std::string m_log_stderr_prefix;

  char *m_log_buf;       ///< coalescing buffer
  size_t m_log_buf_pos;  ///< where we're at within coalescing buffer

  bool m_stop;
  int log_level;

  int m_max_new;

  void *entry() override;

  void _log_safe_write(const char *what, size_t write_len);
  void _flush_logbuf();
  void _flush(std::queue<EntryPtr> *q);

 public:
  Log();
  ~Log() override;

  void set_max_new(int n);
  void set_log_file(std::string fn);
  void reopen_log_file();
  void set_log_stderr_prefix(const std::string &p);

  void flush();

  void set_syslog_level(int log);
  void set_stderr_level(int log);
  void set_log_level(int log);

  EntryPtr create_entry(int level, const char *prefix = nullptr);
  void submit_entry(EntryPtr ep);

  void start();
  void stop();

  /// true if the log lock is held by our thread
  bool is_inside_log_lock();
  static inline int append_time(const Log::log_lock::time_point &t, char *out,
                                size_t outlen);
};
}  // namespace hvs

#endif
