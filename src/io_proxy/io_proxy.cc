/*
 * @Author: Hanjie,Zhou 
 * @Date: 2020-02-20 00:38:30 
 * @Last Modified by: Hanjie,Zhou
 * @Last Modified time: 2020-02-21 22:18:24
 */
#include "io_proxy/io_proxy.h"
#include <pistache/client.h>
#include <boost/filesystem.hpp>
#include <future>
#include <mutex>

// gvds related
#include <experimental/filesystem>
#include "gvds_struct.h"
#include "io_proxy/io_worker.h"
// #include "rpc_bindings.hpp"

namespace gvds {
IOProxyWorker* IOProxy::_get_idle_worker() {
  IOProxyWorker* ret = nullptr;
  // cause io process is fast, use spin lock here
  while (ret == nullptr) {
    idle_list_mu.lock();
    if (idle_list.empty()) {
      idle_list_mu.unlock();
      usleep(1000);
      idle_list_mu.lock();
    } else {
      ret = idle_list.front();
      idle_list.pop();
    }
    idle_list_mu.unlock();
  }
  //  while (!idle_list.pop(ret))
  //    usleep(100);
  idle_worker_num--;
  return ret;
}

bool IOProxy::add_idle_worker(IOProxyWorker* worker) {
  idle_list_mu.lock();
  idle_list.push(worker);
  idle_list_mu.unlock();
  // should not wait, idle list max capcity > max number of idle worker
  //  while (!idle_list.push(wocker))
  //    usleep(100);
  idle_worker_num++;
  return true;
};

bool IOProxy::queue_and_wait(std::shared_ptr<OP> op) {
  auto worker_is_done = make_shared<promise<bool>>();
  auto f = worker_is_done->get_future();
  boost::function0<void> cb = [worker_is_done]() {
    worker_is_done->set_value(true);
  };
  op->complete_callbacks.push_back(cb);
  queue_op(op, true);
  f.wait_for(std::chrono::seconds(3));
}

bool IOProxy::queue_and_wait(const std::vector<std::shared_ptr<OP>>& ops) {
  promise<bool> worker_is_done;
  atomic_long cnt = 0;
  const long expect_done = ops.size();
  auto f = worker_is_done.get_future();
  boost::function0<void> cb = [&worker_is_done, &cnt, expect_done]() {
    int total = ++cnt;
    if (total >= expect_done) {
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
  Node::addr = boost::asio::ip::make_address(ip.value_or("0.0.0.0"));
  auto scher_o = _config->get<int>("ioproxy.scher");
  data_path = _config->get<string>("ioproxy.data_path").value_or("/tmp/data");
  center_id = _config->get<std::string>("ioproxy.cid").value_or("unknown");
  auto port = _config->get<int>("ioproxy.rpc_port").value_or(9092);
  if (!boost::filesystem::exists(data_path.string()) ||
      !boost::filesystem::is_directory(data_path.string())) {
    dout(-1) << "ERROR: IOProxy data path not exists or is not directory."
             << dendl;
    return false;
  }
  proxy_op.data_path = data_path;
  auto __manager_addr = _config->get<std::string>("manager_addr");
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
  // start rpc service
  _ops.reset(new OpServerImpl(port));
  _ops->start();

  // start worker threads and scheduler
  int scher_num = scher_o.value_or(8);
  for (int i = 0; i < scher_num; i++) {
    auto scher = make_shared<IOProxy_scheduler>(true);
    schedulers.push_back(scher);
    for (int j = 0; j < m_max_worker / scher_num; j++) {
      auto worker = scher->create_processor<IOProxyWorker>(_ops->get_service(),
                                                           _ops->get_cq());
      scher->initiate_processor(worker);
    }
    boost::thread* t = new boost::thread(
        boost::bind(&IOProxy_scheduler::operator(), scher.get(), 0));
    worker_threads.push_back(t);
  }
  pthread_mutex_unlock(&m_queue_mutex);

  // _rpc = init_rpcserver();
  // if (!_rpc) {
  //   dout(-1) << "failed to start rpc component, exit!" << dendl;
  //   return false;
  // }
  _udt = init_udtserver();
  if (!_udt) {
    dout(-1) << "failed to start udt component, exit!" << dendl;
    return false;
  }
  repm.start();
  create("io_proxy");
  //  iom.start();
  int max_loop = 3;
  while (!fresh_stat() && max_loop-- > 0) {
    // continue register this ioproxy node to manager node until success
    sleep(5);
  }
  return true;
}

void IOProxy::stop() {
  if (is_started()) {
//    if (_rpc != nullptr) {
//      dout(-1) << "stopping rpc service..." << dendl;
//      _rpc->stop();
//      delete _rpc;
//      _rpc = nullptr;
//    }
    pthread_mutex_lock(&m_queue_mutex);
    m_stop = true;
    pthread_cond_signal(&m_cond_dispatcher);
    pthread_cond_broadcast(&m_cond_ioproxy);
    pthread_mutex_unlock(&m_queue_mutex);
    join();
  }
}

bool IOProxy::fresh_stat() {
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
  //    node.rpc_port = _rpc->port;
  node.rpc_port = _ops->get_port();
  node.data_port = _udt->port;
  node.cid = center_id;
  auto response = client.post(url).body(node.serialize()).send();
  dout(-1) << "DEBUG: Connecting to manager server [" << url << "]" << dendl;

  std::promise<bool> prom;
  auto fu = prom.get_future();
  response.then(
      [&prom](Http::Response res) {
        dout(-1) << "INFO: connected to manager server. Response: "
                 << res.code() << dendl;
        prom.set_value(true);
      },
      Async::IgnoreException);
  auto status = fu.wait_for(std::chrono::seconds(3));
  if (status == std::future_status::timeout) {
    dout(-1) << "WARNING: Lost connection with manager server [" << url << "]"
             << dendl;
    client.shutdown();
    return false;
  }
  promise<bool> prom2;
  auto fu2 = prom2.get_future();
  auto nodelist = client.get(url).send();
  nodelist.then(
          [&prom2, this](Http::Response res) {
              dout(-1) << "INFO: got ioproxy list from manager." << dendl;
              // uuid = res.body();
              map<string, shared_ptr<IOProxyNode>> tmp;
              json_decode(res.body(), tmp);
              for(auto& iop : tmp )
                repm.ioproxys[stoi(iop.second->cid)] = iop.second;
              prom2.set_value(true);
          },
          Async::IgnoreException);
  auto status2 = fu2.wait_for(std::chrono::seconds(10));
  if (status2 == std::future_status::timeout) {
      dout(-1) << "ERROR: cannot get ioproxy list from manager server [" << url << "]"
               << dendl;
      client.shutdown();
      return false;
  }
  client.shutdown();
  return true;
}

void IOProxy::rpc_bind(RpcServer* server) {
  // gvds_ioproxy_rpc_bind(server);
}

gvds::IOProxy* init_ioproxy() {
  gvds::IOProxy* ioproxy = new gvds::IOProxy;
  gvds::HvsContext::get_context()->node = ioproxy;
  if (ioproxy->start())
    return ioproxy;
  else {
    delete ioproxy;
    return nullptr;
  }
}

void destroy_ioproxy(gvds::IOProxy* ioproxy) {
  if (ioproxy) ioproxy->stop();
  //  delete ioproxy;
}
}  // namespace gvds