// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#include "log/Log.h"

#include <errno.h>
#include <string.h>
// #include <syslog.h>
#include <iostream>
#include <context.h>
#include <assert.h>

#include "log/Entry.h"

#define DEFAULT_MAX_NEW 100

#define PREALLOC 1000000
#define MAX_LOG_BUF 65536

namespace hvs {
int Log::append_time(const Log::log_lock::time_point &t, char *out,
                     size_t outlen) {
  auto sec =
      std::chrono::duration_cast<std::chrono::seconds>(t.time_since_epoch())
          .count();
  // auto millisec = std::chrono::duration_cast<std::chrono::milliseconds>(
  auto millisec = std::chrono::duration_cast<std::chrono::microseconds>(
                      t.time_since_epoch() % std::chrono::seconds(1))
                      .count();
  std::tm calendar = {0};
  localtime_r(&sec, &calendar);

  int r;
  // r = std::snprintf(out, outlen, "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
  r = std::snprintf(out, outlen, "%04d-%02d-%02d %02d:%02d:%02d.%06ld",
                    calendar.tm_year + 1900, calendar.tm_mon + 1,
                    calendar.tm_mday, calendar.tm_hour, calendar.tm_min,
                    calendar.tm_sec, static_cast<long>(millisec));
  // Since our caller just adds the return value to something without
  // checking it…
  assert(r >= 0);
  return r;
}

hvs::Log *init_logger() {
  // TODO: use config module to get log path
  auto _config = HvsContext::get_context()->_config;
  auto log_path = _config->get<std::string>("log.path");
  auto log_level = _config->get<int>("log.level");
  if (!log_path) {
    std::cerr << "Log error: invalid log path." << std::endl;
  } else if(!log_level) {
    std::cerr << "Log error: invalid log level." << std::endl;
  } else {
    // success, pass
  }
  hvs::Log *log = new hvs::Log;
  log->set_log_file(*log_path);
  log->set_log_level(*log_level);
  if (!log->reopen_log_file()) {
    return nullptr;
  }
  log->start();
  return log;
}

void stop_log(hvs::Log *log) {
  log->flush();
  log->stop();
  delete log;
}

Log::Log()
    : m_queue_mutex_holder(0),
      m_flush_mutex_holder(0),
      m_new(),
      m_syslog_log(-2),
      m_stderr_log(-1),
      m_log_buf(nullptr),
      m_log_buf_pos(0),
      m_stop(false),
      log_level(10) {
  int ret;

  ret = pthread_mutex_init(&m_flush_mutex, nullptr);
  assert(ret == 0);

  ret = pthread_mutex_init(&m_queue_mutex, nullptr);
  assert(ret == 0);

  ret = pthread_cond_init(&m_cond_loggers, nullptr);
  assert(ret == 0);

  ret = pthread_cond_init(&m_cond_flusher, nullptr);
  assert(ret == 0);

  m_log_buf = (char *)malloc(MAX_LOG_BUF);
}

Log::~Log() {
  flush();
  assert(!is_started());
  m_fstream.close();
  free(m_log_buf);

  pthread_mutex_destroy(&m_queue_mutex);
  pthread_mutex_destroy(&m_flush_mutex);
  pthread_cond_destroy(&m_cond_loggers);
  pthread_cond_destroy(&m_cond_flusher);
}

void Log::set_max_new(int n) { m_max_new = n; }

void Log::set_log_file(std::string fn) { m_log_file = fn; }

void Log::set_log_stderr_prefix(const std::string &p) {
  m_log_stderr_prefix = p;
}

bool Log::reopen_log_file() {
  pthread_mutex_lock(&m_flush_mutex);
  m_flush_mutex_holder = pthread_self();
  if (m_fstream.is_open()) m_fstream.close();
  if (m_log_file.length()) {
    m_fstream.open(m_log_file, std::ios_base::app | std::ios_base::out);
    if (!m_fstream.is_open() || m_fstream.bad()) {
      std::cerr << "failed to open log file " << m_log_file << ": "
                << strerror(errno) << std::endl;
      return false;
    }
  }
  m_flush_mutex_holder = 0;
  pthread_mutex_unlock(&m_flush_mutex);
  return true;
}

void Log::set_syslog_level(int log) {
  pthread_mutex_lock(&m_flush_mutex);
  m_syslog_log = log;
  pthread_mutex_unlock(&m_flush_mutex);
}

void Log::set_stderr_level(int log) {
  pthread_mutex_lock(&m_flush_mutex);
  m_stderr_log = log;
  pthread_mutex_unlock(&m_flush_mutex);
}

void Log::set_log_level(int log) {
  pthread_mutex_lock(&m_flush_mutex);
  log_level = log;
  pthread_mutex_unlock(&m_flush_mutex);
}

void Log::submit_entry(EntryPtr e) {
  e->finish();
  pthread_mutex_lock(&m_queue_mutex);
  m_queue_mutex_holder = pthread_self();

  // wait for flush to catch up
  while (m_new.size() > m_max_new)
    pthread_cond_wait(&m_cond_loggers, &m_queue_mutex);

  m_new.push(e);

  pthread_cond_signal(&m_cond_flusher);
  m_queue_mutex_holder = 0;
  pthread_mutex_unlock(&m_queue_mutex);
}

EntryPtr Log::create_entry(int level, const char *prefix) {
  return std::make_shared<Entry>(
      Entry(clock.now(), pthread_self(), level, prefix));
}

void Log::flush() {
  pthread_mutex_lock(&m_flush_mutex);
  m_flush_mutex_holder = pthread_self();
  pthread_mutex_lock(&m_queue_mutex);
  m_queue_mutex_holder = pthread_self();
  std::queue<EntryPtr> t;
  t.swap(m_new);
  pthread_cond_broadcast(&m_cond_loggers);
  m_queue_mutex_holder = 0;
  pthread_mutex_unlock(&m_queue_mutex);
  _flush(&t);

  m_flush_mutex_holder = 0;
  pthread_mutex_unlock(&m_flush_mutex);
}

void Log::_log_safe_write(const char *what, size_t write_len) {
  if (!m_fstream.is_open()) return;
  m_fstream.write(what, write_len);
  if (m_fstream.bad())
    std::cerr << "problem writing to " << m_log_file << ": " << strerror(errno)
              << std::endl;
}

void Log::_flush_logbuf() {
  if (m_log_buf_pos) {
    _log_safe_write(m_log_buf, m_log_buf_pos);
    m_log_buf_pos = 0;
  }
}

void Log::_flush(std::queue<EntryPtr> *t) {
  EntryPtr e;
  long len = 0;
  while (!t->empty()) {
    e = t->front();
    t->pop();
    assert(e.get());  // EntryPtr shouldn't be empty
    bool should_log = log_level >= e->m_prio;
    bool do_fd = m_fstream.is_open() && should_log;
    bool do_syslog = m_syslog_log >= e->m_prio && should_log;
    bool do_stderr = m_stderr_log >= e->m_prio && should_log;

    if (do_fd || do_syslog || do_stderr) {
      size_t line_used = 0;

      char *line;
      size_t line_size = 80 + e->size();
      bool need_dynamic = line_size >= MAX_LOG_BUF;

      // this flushes the existing buffers if either line is longer
      // than our buffer, or buffer is too full to fit it
      if (m_log_buf_pos + line_size >= MAX_LOG_BUF) {
        _flush_logbuf();
      }
      if (need_dynamic) {
        line = new char[line_size];
      } else {
        line = &m_log_buf[m_log_buf_pos];
      }
      line_used +=
          append_time(e->m_stamp, line + line_used, line_size - line_used);
      line_used += snprintf(line + line_used, line_size - line_used,
                            " %lx %2d ", (unsigned long)e->m_thread, e->m_prio);

      line_used += e->snprintf(line + line_used, line_size - line_used - 1);
      assert(line_used < line_size);

      if (do_syslog) {
        // syslog(LOG_USER, "%s", line);
        // avoid syslog
      }

      if (do_stderr) {
        line[line_used] = 0;
        std::cerr << m_log_stderr_prefix << line << std::endl;
      }

      if (do_fd) {
        line[line_used] = '\n';
        if (need_dynamic) {
          _log_safe_write(line, line_used + 1);
          m_log_buf_pos = 0;
        } else {
          m_log_buf_pos += line_used + 1;
        }
      } else {
        m_log_buf_pos = 0;
      }

      if (need_dynamic) {
        delete[] line;
      }
    }
  }

  _flush_logbuf();
}

void Log::start() {
  assert(!is_started());
  pthread_mutex_lock(&m_queue_mutex);
  m_stop = false;
  pthread_mutex_unlock(&m_queue_mutex);
  create("log");
}

void Log::stop() {
  if (is_started()) {
    pthread_mutex_lock(&m_queue_mutex);
    m_stop = true;
    pthread_cond_signal(&m_cond_flusher);
    pthread_cond_broadcast(&m_cond_loggers);
    pthread_mutex_unlock(&m_queue_mutex);
    join();
  }
}

void *Log::entry() {
  pthread_mutex_lock(&m_queue_mutex);
  m_queue_mutex_holder = pthread_self();
  while (!m_stop) {
    if (!m_new.empty()) {
      m_queue_mutex_holder = 0;
      pthread_mutex_unlock(&m_queue_mutex);
      flush();
      pthread_mutex_lock(&m_queue_mutex);
      m_queue_mutex_holder = pthread_self();
      continue;
    }

    pthread_cond_wait(&m_cond_flusher, &m_queue_mutex);
  }
  m_queue_mutex_holder = 0;
  pthread_mutex_unlock(&m_queue_mutex);
  flush();
  return NULL;
}

bool Log::is_inside_log_lock() {
  return pthread_self() == m_queue_mutex_holder ||
         pthread_self() == m_flush_mutex_holder;
}

}  // namespace hvs
