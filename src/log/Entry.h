// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#pragma once

#include <assert.h>
#include <pthread.h>
#include <chrono>
#include <memory>
#include <sstream>
#include <cstring>

namespace hvs {
class Entry {
 public:
  using log_time = std::chrono::time_point<std::chrono::system_clock>;
  log_time m_stamp;
  pthread_t m_thread;
  int m_prio;

  std::string m_buf;  // use raw type to provide more tool function
  std::shared_ptr<std::stringstream> m_stream_p;

  Entry() : Entry(log_time{}, 0, 0) {}
  Entry(log_time s, pthread_t t, int level, const char *prefix = nullptr)
      : m_stamp(s), m_thread(t), m_prio(level) {
    m_stream_p = std::make_shared<std::stringstream>(std::stringstream());
    if (prefix) {
      *m_stream_p << prefix << ' ';
    }
  }
  ~Entry() = default;

 public:
  std::ostream &get_ostream() {
    assert(m_stream_p.get());
    return *m_stream_p;
  }

  void set_str(const std::string &s) { get_ostream() << s; }

  std::string get_str() {
    if (m_stream_p.get())
      return m_stream_p->str();
    else
      // m_stream has been finished
      return m_buf;
  }

  // Entry's m_stream would exist in a thread until function finish being called
  void finish() {
    m_buf = get_str();
    m_stream_p.reset();
  }

  size_t size() const { return m_buf.size(); }

  size_t snprintf(char *dst, size_t max_n) const {
    memcpy(dst, m_buf.c_str(), m_buf.size() > max_n ? max_n : m_buf.size());
    return m_buf.size() > max_n ? max_n : m_buf.size();
  }
};

using EntryPtr = std::shared_ptr<Entry>;
}  // namespace hvs
