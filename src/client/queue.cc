#include "client/queue.h"
#include <future>
#include <mutex>
#include "client/client_worker.h"

using namespace std;
using namespace hvs;

#define MAX_MERGE_SIZE 1048576
namespace hvs {
ClientWorker* ClientBufferQueue::_get_idle_worker() {
  ClientWorker* ret;
  // cause io process is fast, use spin lock here
  while (!idle_list.pop(ret))
    usleep(10);
  idle_worker_num--;
  return ret;
}

bool ClientBufferQueue::add_idle_worker(ClientWorker* wocker) {
  // should not wait, idle list max capcity > max number of idle worker
  while (!idle_list.push(wocker))
    usleep(10);
  idle_worker_num++;
  return true;
};

// producer buf
bool ClientBufferQueue::queue_buffer(std::shared_ptr<Buffer> buf, bool block) {
  pthread_mutex_lock(&m_queue_mutex);
  m_queue_mutex_holder = pthread_self();

  // see if we can merge
      if (!buf_waiting_line.empty()){
        auto& last_buf = buf_waiting_line.back();
        // if buf satisfies the following conditions, then do merge
        // 1. the same path with last queued buf
        // 2. consequent
        // 3. the merged size is smaller than 1MB
        if(last_buf && last_buf->path == buf->path && last_buf->buf.size + buf->buf.size < MAX_MERGE_SIZE &&
           last_buf->offset+last_buf->buf.size == buf->offset) {
          // do merge
          last_buf->append(buf->buf);
          buf->destroy();
          pthread_cond_signal(&m_cond_dispatcher);
          m_queue_mutex_holder = 0;
          pthread_mutex_unlock(&m_queue_mutex);
          return true;
        }
      }
  // wait for dispatch to catch up
  while (buf_waiting_line.size() + buf_onlink > m_max_buf && block)
    pthread_cond_wait(&m_cond_ioproxy, &m_queue_mutex);

  buf_onlink++;
  buf_waiting_line.push(buf);
  pthread_cond_signal(&m_cond_dispatcher);
  m_queue_mutex_holder = 0;
  pthread_mutex_unlock(&m_queue_mutex);
  return true;
}

void* ClientBufferQueue::entry() {
  pthread_mutex_lock(&m_queue_mutex);
  m_queue_mutex_holder = pthread_self();
  while (!m_stop) {
    if (!buf_waiting_line.empty()) {
      m_queue_mutex_holder = 0;
      pthread_mutex_unlock(&m_queue_mutex);
      _dispatch();
      pthread_mutex_lock(&m_queue_mutex);
      m_queue_mutex_holder = pthread_self();
      continue;
    }
    pthread_cond_wait(&m_cond_dispatcher, &m_queue_mutex);
  }
  m_queue_mutex_holder = 0;
  pthread_mutex_unlock(&m_queue_mutex);
}

void ClientBufferQueue::done_one() {
  pthread_mutex_lock(&m_queue_mutex);
  m_queue_mutex_holder = pthread_self();
  buf_onlink--;
  pthread_cond_broadcast(&m_cond_ioproxy);
  m_queue_mutex_holder = 0;
  pthread_mutex_unlock(&m_queue_mutex);
  ASSERT(buf_onlink >= 0, "ERROR: Client onlink buffer won't less than 0");
}

// consumer buf
void ClientBufferQueue::_dispatch() {
  pthread_mutex_lock(&m_dispatch_mutex);
  m_dispatcher_mutex_holder = pthread_self();

  pthread_mutex_lock(&m_queue_mutex);
  m_queue_mutex_holder = pthread_self();
  std::queue<std::shared_ptr<Buffer>> t;
  t.swap(buf_waiting_line);
  pthread_cond_broadcast(&m_cond_ioproxy);
  m_queue_mutex_holder = 0;
  pthread_mutex_unlock(&m_queue_mutex);
  // do dispatch
  _dispatch_unsafe(&t);

  m_dispatcher_mutex_holder = 0;
  pthread_mutex_unlock(&m_dispatch_mutex);
}

void ClientBufferQueue::_dispatch_unsafe(
    std::queue<std::shared_ptr<Buffer>>* t) {
  // declare release object
  while (!t->empty()) {
    auto buf = t->front();
    t->pop();
    assert(buf.get());                 // buf ptr should not be empty
    auto worker = _get_idle_worker();  // may wait on spin lock
    boost::intrusive_ptr<BufferQueued> bufq = new BufferQueued(buf);
    worker->my_scheduler().queue_event(worker->my_handle(), bufq);
  }
}

void ClientBufferQueue::start() {
  pthread_mutex_lock(&m_queue_mutex);
  m_stop = false;

  int scher_num = 8;
  for (int i = 0; i < scher_num; i++) {
    auto scher = make_shared<Client_scheduler>(true);
    schedulers.push_back(scher);
    for (int j = 0; j < m_max_worker / scher_num; j++) {
      auto worker = scher->create_processor<ClientWorker>();
      scher->initiate_processor(worker);
    }
    boost::thread* t = new boost::thread(
        boost::bind(&Client_scheduler::operator(), scher.get(), 0));
    worker_threads.push_back(t);
  }
  pthread_mutex_unlock(&m_queue_mutex);

  create("client_buf_queue");
}

void ClientBufferQueue::stop() {
  if (is_started()) {
    pthread_mutex_lock(&m_queue_mutex);
    m_stop = true;
    pthread_cond_signal(&m_cond_dispatcher);
    pthread_cond_broadcast(&m_cond_ioproxy);
    pthread_mutex_unlock(&m_queue_mutex);
    join();
  }
}
}  // namespace hvs