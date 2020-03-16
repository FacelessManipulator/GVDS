/*
 * @Author: Hanjie,Zhou
 * @Date: 2020-02-21 15:32:17
 * @Last Modified by: Hanjie,Zhou
 * @Last Modified time: 2020-02-21 15:45:55
 */
#pragma once
#include <atomic>
#include <chrono>
#include <experimental/filesystem>
#include <functional>
#include <mutex>
#include <string>
#include "boost/asio/ip/address.hpp"
#include "common/Thread.h"
#include "hvs_struct.h"
#include "msg/operator_client.hpp"
#include "op.pb.h"

#define _GVDS_SPACE_REPLICA_FILE_INDEX "user.gvds.r.fi"
#define _GVDS_FILE_INDEX_WRITE 0x1
#define _GVDS_FILE_INDEX_SYNC 0x2
#define _GVDS_FILE_INDEX_CHECK 0x4


namespace fs = std::experimental::filesystem;
namespace hvs {
class IOProxy;
class ReplicaMgr : public Thread {
 private:
  int m_stop;

 public:
  std::map<uint32_t, std::shared_ptr<IOProxyNode>> ioproxys;

 public:
  ReplicaMgr(IOProxy* ioProxy)
      : iop(ioProxy),
        m_queue_mutex_holder(0),
        m_flush_mutex_holder(0),
        m_max_new(1024 * 1024) {
    int ret;
    ret = pthread_mutex_init(&m_flush_mutex, nullptr);
    assert(ret == 0);
    ret = pthread_mutex_init(&m_queue_mutex, nullptr);
    assert(ret == 0);
    ret = pthread_cond_init(&m_cond_loggers, nullptr);
    assert(ret == 0);
    ret = pthread_cond_init(&m_cond_flusher, nullptr);
    assert(ret == 0);
  }
  void start();
  void* entry() override;
  void stop();
  std::shared_ptr<OperatorClient> get_operator(
      std::shared_ptr<IOProxyNode> node, int channel_id);
  // this function usually called after request has properly handled and reach
  // its end of life in local ioproxy so I use move semantic to cut down copy
  // cost
  void handle_replica_async(OpRequest&& request, OpReply& reply);
  int create_replicated_space(const std::string& filepath,
                              const std::string& cids);
  // this function handle the situation that the dest extent of file data was stored in the remote
  // center and ioproxy have to sync the specific area before providing the data.
  int sync_filedata(const OpRequest& request);
  int sync_filedata(const std::string& path, uint64_t off, uint64_t len);
  int sync_unalign_page(const std::string& path, uint64_t off, uint64_t len);

 private:
  void flush();
  void _flush(std::queue<std::shared_ptr<OpRequest>>* q);
  void submit_replicated_request(std::shared_ptr<OpRequest>& request);
  std::shared_ptr<OpRequest> _replicate_space_request(fs::path space, const std::string& cids);
  void _handle_create_replicated_space(const OpRequest& request,
                                       OpReply& reply);
  void _handle_file_index_update(const OpRequest& request, OpReply& reply);
  void _handle_file_sync_data(const OpRequest& request, OpReply& reply);

 private:
  IOProxy* iop;
  uint8_t mcid;
  fs::path replicate_data_path;
  std::unordered_map<std::string, std::shared_ptr<OperatorClient>> operators;
  std::mutex mu_;
  pthread_mutex_t m_queue_mutex;
  ///< 日志提交队列m_new的锁
  pthread_mutex_t m_flush_mutex;
  ///< 日志flush的锁，用来互斥操作文件和冲刷日志的操作
  pthread_cond_t m_cond_loggers;
  ///< 当日志队列满的时候用来让其他线程等待的信号量，可能会阻塞调用dout的线程
  pthread_cond_t m_cond_flusher;
  ///< 当日志队列空的时候用来让日志线程等待的信号量

  pthread_t m_queue_mutex_holder;
  ///< 用来记录队列锁的持有者，用来让其他线程判断自己持有log锁
  pthread_t m_flush_mutex_holder;
  ///< 用来记录冲刷锁的持有者，用来让其他线程判断自己持有log锁

  std::queue<std::shared_ptr<OpRequest>> m_new;  ///< 日志的记录队列
  int m_max_new;  ///< 日志队列的最大记录数
};
};  // namespace hvs