#include "io_proxy/io_proxy.h"
#include <pistache/client.h>
#include <boost/filesystem.hpp>
#include <future>
#include <mutex>
#include "hvs_struct.h"
#include "io_proxy/io_worker.h"
#include "rpc_bindings.hpp"

using namespace std;
using namespace hvs;
using namespace Pistache;

namespace hvs {
IOProxyWorker* IOProxy::_get_idle_worker() {
  IOProxyWorker* ret;
  // cause io process is fast, use spin lock here
  while (!idle_list.pop(ret))
    ;
  return ret;
}

bool IOProxy::add_idle_worker(IOProxyWorker* woker) {
  // should not wait, idle list max capcity > max number of idle worker
  while (!idle_list.push(woker))
    ;
};

bool IOProxy::queue_and_wait(std::shared_ptr<OP> op) {
  promise<bool> worker_is_done;
  auto f = worker_is_done.get_future();
  boost::function0<void> cb = [&worker_is_done]() {
    worker_is_done.set_value(true);
  };
  op->complete_callbacks.push_back(cb);
  queue_op(op, true);
  f.wait();
  // WARNING: DO NOT CHANGE THE COMPLETE_CALLBACKS DURING THIS PERIODS
}

bool IOProxy::queue_and_wait(const std::vector<std::shared_ptr<OP>>& ops) {
  promise<bool> worker_is_done;
  atomic_long cnt = 0;
  const long expect_done = ops.size();
  auto f = worker_is_done.get_future();
  boost::function0<void> cb = [&worker_is_done, &cnt, expect_done]() {
    ++cnt;
    if (cnt.load() >= expect_done) {
      worker_is_done.set_value(true);
    }
  };
  for (auto op : ops) {
    op->complete_callbacks.push_back(cb);
    queue_op(op, true);
  }
  f.wait_for(std::chrono::milliseconds(100) * ops.size());
  // WARNING: DO NOT CHANGE THE COMPLETE_CALLBACKS DURING THIS PERIODS
}

// producer op
bool IOProxy::queue_op(std::shared_ptr<OP> op, bool block) {
  pthread_mutex_lock(&m_queue_mutex);
  m_queue_mutex_holder = pthread_self();

  // wait for dispatch to catch up
  if (op_waiting_line.size() > m_max_op && !block) return false;
  while (op_waiting_line.size() > m_max_op && block)
    pthread_cond_wait(&m_cond_ioproxy, &m_queue_mutex);

  op_waiting_line.push(op);
  op->op_queued = std::chrono::steady_clock::now();

  pthread_cond_signal(&m_cond_dispatcher);
  m_queue_mutex_holder = 0;
  pthread_mutex_unlock(&m_queue_mutex);
  return true;
}

void* IOProxy::entry() {
  pthread_mutex_lock(&m_queue_mutex);
  m_queue_mutex_holder = pthread_self();
  while (!m_stop) {
    if (!op_waiting_line.empty()) {
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

// consumer op
void IOProxy::_dispatch() {
  pthread_mutex_lock(&m_dispatch_mutex);
  m_dispatcher_mutex_holder = pthread_self();

  pthread_mutex_lock(&m_queue_mutex);
  m_queue_mutex_holder = pthread_self();
  std::queue<std::shared_ptr<OP>> t;
  t.swap(op_waiting_line);
  pthread_cond_broadcast(&m_cond_ioproxy);
  m_queue_mutex_holder = 0;
  pthread_mutex_unlock(&m_queue_mutex);
  // do dispatch
  _dispatch_unsafe(&t);

  m_dispatcher_mutex_holder = 0;
  pthread_mutex_unlock(&m_dispatch_mutex);
}

void IOProxy::_dispatch_unsafe(std::queue<std::shared_ptr<OP>>* t) {
  // declare release object
  //  std::shared_ptr<OP> op;
  while (!t->empty()) {
    auto op = t->front();
    t->pop();
    assert(op.get());                  // op ptr should not be empty
    auto worker = _get_idle_worker();  // may wait on spin lock
    boost::intrusive_ptr<OpQueued> opq = new OpQueued(op);
    worker->my_scheduler().queue_event(worker->my_handle(), opq);
  }
}

bool IOProxy::start() {
  pthread_mutex_lock(&m_queue_mutex);
  m_stop = false;
  // TODO: read configs
  auto _config = HvsContext::get_context()->_config;
  auto ip = _config->get<string>("ip");
  Node::addr =  boost::asio::ip::make_address(ip.value_or("0.0.0.0"));
  auto scher_o = _config->get<int>("ioproxy.scher");
  data_path = _config->get<string>("ioproxy.data_path").value_or("/tmp/data");
  if (!boost::filesystem::exists(data_path) ||
      !boost::filesystem::is_directory(data_path)) {
    dout(-1) << "ERROR: IOProxy data path not exists or is not directory."
             << dendl;
    return false;
  }
  auto __manager_addr = _config->get<std::string>("ioproxy.manager_addr");
  if (!__manager_addr) {
    dout(-1) << "ERROR: NO MANAGER ADDR FOUND IN CONFIG FILE!\nPlease add "
                "manager addr in config file."
             << dendl;
    // may test with proxy only mode.
    // exit(-1);
  } else {
    manager_addr = __manager_addr.value();
  }
  if (!_config->get<string>("ioproxy.uuid").has_value()) {
    dout(-1) << "please use linux command UUID to generate IO proxy's UUID and "
                "insert it into config file."
             << dendl;
    return false;
  }
  uuid = _config->get<string>("ioproxy.uuid").value();

  int scher_num = scher_o.value_or(8);
  for (int i = 0; i < scher_num; i++) {
    auto scher = make_shared<IOProxy_scheduler>(true);
    schedulers.push_back(scher);
    for (int j = 0; j < m_max_worker / scher_num; j++) {
      auto worker = scher->create_processor<IOProxyWorker>();
      scher->initiate_processor(worker);
    }
    boost::thread* t = new boost::thread(
        boost::bind(&IOProxy_scheduler::operator(), scher.get(), 0));
    worker_threads.push_back(t);
  }
  pthread_mutex_unlock(&m_queue_mutex);

  _rpc = init_rpcserver();
  if (!_rpc) {
    dout(-1) << "failed to start rpc component, exit!" << dendl;
    return false;
  }
  _udt = init_udtserver();
  if (!_udt) {
    dout(-1) << "failed to start udt component, exit!" << dendl;
    return false;
  }
  create("io_proxy");
  fresh_stat();
  return true;
}

void IOProxy::stop() {
  if (is_started()) {
    if (_rpc != nullptr) {
      dout(-1) << "stopping rpc service..." << dendl;
      _rpc->stop();
      delete _rpc;
      _rpc = nullptr;
    }
    pthread_mutex_lock(&m_queue_mutex);
    m_stop = true;
    pthread_cond_signal(&m_cond_dispatcher);
    pthread_cond_broadcast(&m_cond_ioproxy);
    pthread_mutex_unlock(&m_queue_mutex);
    join();
  }
}

void IOProxy::fresh_stat() {
  // regist itself to manager node
  Http::Client client;
  char url[256];
  snprintf(url, 256, "http://%s/ioproxy", manager_addr.c_str());
  auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
  client.init(opts);

  // add ioproxy
  IOProxyNode node;
  node.uuid = uuid;
  node.name = Node::name;
  node.ip = Node::addr.to_string();
  node.rpc_port = _rpc->port;
  node.data_port = _udt->port;
  auto response = client.post(url).body(node.serialize()).send();
  dout(-1) << "DEBUG: Connecting to manager server [" << url << "]" << dendl;

  std::promise<bool> prom;
  auto fu = prom.get_future();
  response.then(
      [&](Http::Response res) {
        dout(10) << "INFO: connected to manager server. Response: " << res.code()
                 << dendl;
        // uuid = res.body();
        prom.set_value(true);
      },
      Async::IgnoreException);
  auto status = fu.wait_for(std::chrono::seconds(3));
  if(status == std::future_status::timeout)
    dout(-1) << "WARNING: Lost connection with manager server [" << url << "]" << dendl;
  client.shutdown();
}

void IOProxy::rpc_bind(RpcServer* server) { hvs_ioproxy_rpc_bind(server); }

hvs::IOProxy* init_ioproxy() {
  hvs::IOProxy* ioproxy = new hvs::IOProxy;
  hvs::HvsContext::get_context()->node = ioproxy;
  if(ioproxy->start())
    return ioproxy;
  else {
    delete ioproxy;
    return nullptr;
  }
}

void destroy_ioproxy(hvs::IOProxy* ioproxy) {
  if(ioproxy)
    ioproxy->stop();
  //  delete ioproxy;
}
}  // namespace hvs