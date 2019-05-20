#include "msg/udt_writer.h"
#include "context.h"
using namespace hvs;
using namespace std;

void UDTWriter::start() {
  m_max_write = 100;
  m_stop = false;
  create("udt session");
}

void *UDTWriter::entry() {
  pthread_mutex_lock(&m_queue_mutex);
  while (!m_stop) {
    if (!m_new.empty()) {
      pthread_mutex_unlock(&m_queue_mutex);
      do_write();
      pthread_mutex_lock(&m_queue_mutex);
      continue;
    }
    pthread_cond_wait(&m_cond_writer, &m_queue_mutex);
  }
  pthread_mutex_unlock(&m_queue_mutex);
  do_write();
  return NULL;
}

void UDTWriter::close() {
  if (!m_stop) {
    pthread_mutex_lock(&m_queue_mutex);
    m_stop = true;
    pthread_cond_signal(&m_cond_writer);
    pthread_cond_broadcast(&m_cond_session);
    pthread_mutex_unlock(&m_queue_mutex);
    join();
  }
}

void UDTWriter::write(clmdep_msgpack::sbuffer &&data) {
  pthread_mutex_lock(&m_queue_mutex);
  while (m_new.size() > m_max_write) {
    // buffer size exceed limit, wait on session's cond
    pthread_cond_wait(&m_cond_session, &m_queue_mutex);
  }
  m_new.push(std::move(data));
  pthread_cond_signal(&m_cond_writer);
  pthread_mutex_unlock(&m_queue_mutex);
}

void UDTWriter::do_write() {
  pthread_mutex_lock(&m_writer_mutex);
  pthread_mutex_lock(&m_queue_mutex);
  std::queue<clmdep_msgpack::sbuffer> t;
  t.swap(m_new);
  // buffer consumed.
  pthread_cond_broadcast(&m_cond_session);
  pthread_mutex_unlock(&m_queue_mutex);
  _write_unsafe(&t);

  pthread_mutex_unlock(&m_writer_mutex);
}

void UDTWriter::_write_unsafe(std::queue<clmdep_msgpack::sbuffer> *q) {
  while (!q->empty()) {
    // zero copy with move semantic
    clmdep_msgpack::sbuffer data(std::move(q->front()));
    q->pop();
    {
      // send circle in case of the lack of udt buffer
      unsigned long sent = 0, total = data.size();
      while (sent < total) {
        long ss = UDT::send(socket_, data.data(), data.size(), 0);
        if (ss == UDT::ERROR) {
          dout(5) << "WARNING: send error occured: "
                  << UDT::getlasterror().getErrorMessage() << dendl;
          break;
        }
        sent += ss;
      }
      if (sent < total) {
        // error occured, close session
        m_stop = true;
        break;
      }
    }
  }
}