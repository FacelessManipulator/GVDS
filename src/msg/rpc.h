#pragma once
#include <memory>
#include <optional>
#include <string>
#include <unistd.h>
#include "context.h"
#include "common/debug.h"

// avoid config with sys/syslog.h
// #undef LOG_INFO
// #undef LOG_DEBUG
#include "rpc/client.h"
#include "rpc/server.h"
#include "rpc/rpc_error.h"
// #undef LOG_INFO
// #undef LOG_DEBUG
// #define LOG_INFO 6
// #define LOG_DEBUG 7

namespace hvs {
class RpcServer {
 public:
  RpcServer() = delete;
  RpcServer(std::string& _ip, unsigned _port) : ip(_ip), port(_port) {
    _server = std::make_unique<rpc::server>(_ip, _port);
    // avoid all excepotions
    _server->suppress_exceptions(true);
  }
  ~RpcServer() {
    if (_server) stop();
  }
  template <class F>
  void bind(std::string path, F func) {
    _server->bind(path, func);
  }
  void run(unsigned threads) { _server->async_run(threads); }
  void stop() {
    if (_server) {
      // TODO: BUG inside rpclib! close_sessions may trigger the aync close after clear session-vector
      // which is an undefined behavior and would throw exception.
//      usleep(50);
//      _server->close_sessions();
      _server->stop();
      _server.reset();
    }
  }

 public:
  const std::string ip;
  const uint16_t port;

 private:
  std::unique_ptr<rpc::server> _server;
};
RpcServer* init_rpcserver();
class RpcClient {
 public:
  RpcClient(const std::string address, const unsigned port);
  RpcClient() {};
  template <typename... Args>
  std::optional<RPCLIB_MSGPACK::object_handle> call(
      std::string const& func_name, Args... args);
  void shutdown() {}

 public:
  std::shared_ptr<rpc::client> _client;
  std::string _address;
  std::uint16_t _port;
  unsigned _retry;
};

template <typename... Args>
std::optional<RPCLIB_MSGPACK::object_handle> RpcClient::call(
    std::string const& func_name, Args... args) {
  int retry_times = 0;
  do {
    ++retry_times;
    try {
      auto obj = _client->call(func_name, args...);
      return move(obj);
    } catch (rpc::timeout timeout) {
      dout(5) << "WARING: rpc client timeout, try " << retry_times << " time."
              << dendl;
    } catch (rpc::rpc_error error) {
      dout(-1) << "ERROR: rpc client call " << error.get_function_name()
              << " error, reason: " << error.what() << dendl;
      return {};
    }
  } while (retry_times < _retry);
  dout(1) << "ERROR: rpc client get no response, max retries=" << _retry
          << dendl;
  return {};
}

}  // namespace hvs