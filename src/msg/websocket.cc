#include "msg/websocket.h"

using namespace hvs;
using namespace std;

void fail(boost::system::error_code ec, char const* what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

WebsocketMessager::WebsocketMessager(std::string& _addr, unsigned short _port,
                                     int threads)
    : port(_port), ioc(threads), threads_num(threads) {
  addr = boost::asio::ip::make_address(_addr);
}

void WebsocketMessager::run() {
  // start listener
  listener = make_shared<WebsocketListener>(
      ioc, boost::asio::ip::tcp::endpoint{addr, port});
  listener->run();
  threads.reserve(threads_num);
  for (int i = 0; i < threads_num; i++) {
    threads.emplace_back([this]() { ioc.run(); });
  }
}

bool WebsocketClient::connect() {
  
}

void WebsocketClient::async_write(
    const char* buf, size_t size,
    std::optional<std::function<void(boost::system::error_code, size_t)>>
        callback) {
  auto buf = boost::asio::buffer(buf, size);
  auto callback_real =
      callback.value_or([](boost::system::error_code e, size_t s) {});
  _ws.async_write(buf, callback_real);
};

bool WebsocketClient::write(const char* buf, size_t size, unsigned timeout) {
  std::promise<boost::system::error_code> _promise;
  auto ft = _promise.get_future();
  auto callback = [&_promise](boost::system::error_code e, size_t s) {
    _promise.set_value(e);
  };
  auto res = ft.wait_until(std::chrono::steady_clock::now() +
                           std::chrono::seconds(timeout));
  if (res == std::future_status::ready) {
    auto err = ft.get();
    if (err == boost::system::errc::success)
      return true;
    else {
      dout(5) << "WebsocketClient: send request timeout. { host = " << host
              << "; port= " << port << "; reason= " << err.message() << "; }"
              << dendl;
      return false;
    }
  } else {
    dout(5) << "WebsocketClient: send request timeout. { host = " << host
            << "; port= " << port << "; timeout= " << timeout << "s; }"
            << dendl;
    return false;
  }
}

void WebsocketSession::run() {
  // Accept the websocket handshake
  _ws.async_accept(boost::asio::bind_executor(
      _strand, std::bind(&WebsocketSession::on_accept, shared_from_this(),
                         std::placeholders::_1)));
}

void WebsocketSession::on_accept(boost::system::error_code ec) {
  if (ec) return fail(ec, "accept");

  // Read a message
  read();
}

void WebsocketSession::on_read(boost::system::error_code ec,
                               size_t bytes_transferred) {
  // This indicates that the session was closed
  if (ec == beast::websocket::error::closed) return;
  dout(-1) << "receive msg" << dendl;

  if (ec) fail(ec, "read");
  // TODO: handle the message
  sleep(1);  // assume we cost 1s to handle the msg
  for (auto it : _rbuffer.data()) {
    it.data();
  }
  _rbuffer.consume(0);
}

void WebsocketSession::on_write(boost::system::error_code ec,
                                std::size_t bytes_transferred) {
  if (ec) return fail(ec, "write");

  // Clear the buffer
  _rbuffer.consume(_rbuffer.size());
  // Do another read
  read();
}

void WebsocketSession::read() {
  // Read a message into our buffer
  _ws.async_read(
      _rbuffer,
      boost::asio::bind_executor(
          _strand, std::bind(&WebsocketSession::on_read, shared_from_this(),
                             std::placeholders::_1, std::placeholders::_2)));
}

void WebsocketSession::write() {
  // currently we only write binary data
  _ws.text(false);
  _ws.async_write(
      _wbuffer.data(),
      boost::asio::bind_executor(
          _strand, std::bind(&WebsocketSession::on_write, shared_from_this(),
                             std::placeholders::_1, std::placeholders::_2)));
}

WebsocketListener::WebsocketListener(boost::asio::io_context& ioc,
                                     boost::asio::ip::tcp::endpoint endpoint)
    : _acceptor(ioc), _socket(ioc) {
  boost::system::error_code ec;

  // Open the acceptor
  _acceptor.open(endpoint.protocol(), ec);
  if (ec) {
    fail(ec, "open");
    return;
  }

  // Allow address reuse
  _acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
  if (ec) {
    fail(ec, "set_option");
    return;
  }

  // Bind to the server address
  _acceptor.bind(endpoint, ec);
  if (ec) {
    fail(ec, "bind");
    return;
  }

  // Start listening for connections
  _acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
  if (ec) {
    fail(ec, "listen");
    return;
  }
}

void WebsocketListener::run() {
  if (!_acceptor.is_open()) return;
  do_accept();
}

void WebsocketListener::do_accept() {
  _acceptor.async_accept(
      _socket, std::bind(&WebsocketListener::on_accept, shared_from_this(),
                         std::placeholders::_1));
}

void WebsocketListener::on_accept(boost::system::error_code ec) {
  if (ec) {
    fail(ec, "accept");
  } else {
    // Create the session and run it
    std::make_shared<WebsocketSession>(std::move(_socket))->run();
  }

  // Accept another connection
  do_accept();
}
