#pragma once

#include <algorithm>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "context.h"
#include "msg/dispatcher.h"

namespace gvds {
namespace beast = ::boost::beast;
class WebsocketListener;
class WebsocketSession;

class WebsocketMessager {
 public:
  std::shared_ptr<WebsocketListener> listener;
  std::vector<std::thread> threads;
  int threads_num;
  boost::asio::io_context ioc;
  boost::asio::ip::address addr;
  unsigned short port;

  WebsocketMessager(std::string& address, unsigned short port, int threads);
  void run();

 private:
  void fail(beast::error_code ec, char const* what);
};

class WebsocketClient {
 public:
  WebsocketClient(std::string _host, unsigned short _port)
      : host(_host), port(_port), _resolver(ioc), _ws(ioc){};
  bool connect();
  void async_write(
      const char* buf, size_t size,
      std::optional<std::function<void(boost::system::error_code, size_t)>>
          callback);
  bool write(const char* buf, size_t size, unsigned timeout = 3);
  void async_read();

 private:
  boost::asio::io_context ioc;
  boost::asio::ip::tcp::resolver _resolver;
  beast::websocket::stream<boost::asio::ip::tcp::socket> _ws;
  beast::multi_buffer _buffer;
  std::string host;
  unsigned short port;
};

class WebsocketSession : public std::enable_shared_from_this<WebsocketSession> {
  beast::websocket::stream<boost::asio::ip::tcp::socket> _ws;
  boost::asio::strand<boost::asio::io_context::executor_type> _strand;
  boost::beast::multi_buffer _rbuffer;  // read buffer
  boost::beast::multi_buffer _wbuffer;  // write buffer

 public:
  // Take ownership of the socket
  explicit WebsocketSession(boost::asio::ip::tcp::socket socket)
      : _ws(std::move(socket)), _strand(_ws.get_executor()) {}

  // Start the asynchronous operation
  void run();

  void on_accept(boost::system::error_code ec);
  void on_read(boost::system::error_code ec, size_t bytes_transferred);
  void on_write(boost::system::error_code ec, size_t bytes_transferred);
  void read();
  void write();
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class WebsocketListener
    : public std::enable_shared_from_this<WebsocketListener> {
  boost::asio::ip::tcp::acceptor _acceptor;
  boost::asio::ip::tcp::socket _socket;

 public:
  WebsocketListener(boost::asio::io_context& ioc,
                    boost::asio::ip::tcp::endpoint endpoint);

  // Start accepting incoming connections
  void run();

  void do_accept();

  void on_accept(boost::system::error_code ec);
};

};  // namespace gvds