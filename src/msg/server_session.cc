#include "msg/server_session.h"
#include "context.h"
#include "io_proxy/rpc_bindings.hpp"
#include "io_proxy/rpc_types.h"
#include "msg/udt_server.h"

using namespace hvs;
using namespace std;
UDTSession::UDTSession(UDTServer *srv, UDTSOCKET socket)
    : parent(srv), unpacker(), socket_(socket), m_stop(false) {
  unpacker.reserve_buffer(102400);
}

void UDTSession::start() {}

void UDTSession::close() {
  m_stop = true;
  // close udt socket
  //   write_strand_.post([this]() {
  //     socket_.close();
  //     parent_->close_session(shared_from_base<server_session>());
  //   });
}

// currently without asio, we use epoll.
// do_read would be called when epoll returned. session should copy buffers
// from UDT and return as quick as possible
void UDTSession::do_read() {
  constexpr std::size_t max_read_bytes = 102400;
  constexpr std::size_t default_buffer_size = max_read_bytes;

  unsigned long rs = 0;
  //   UDT::getsockopt(socket_, 0, UDT_RCVDATA, &rcv_size, &var_size);
  if (UDT::ERROR ==
      (rs = UDT::recv(socket_, unpacker.buffer(), default_buffer_size, 0))) {
    dout(10) << "WARNING: recv error:" << UDT::getlasterror().getErrorMessage()
             << dendl;
    // maybe close session?
    return;
  } else {
    unpacker.buffer_consumed(rs);
  }

  // function should return immediately, async op instead.
  // assemble the data op call
  clmdep_msgpack::unpacked result;
  while (unpacker.next(result) && !m_stop) {
    clmdep_msgpack::object_handle
    auto msg = result.get();
    // handle the message
    //   auto resp = handle(msg);
    try {
      auto buf = msg.as<ioproxy_rpc_buffer>();
      auto op = std::make_shared<IOProxyDataOP>();
      op->path.assign("/tmp/hvs/data/");
      op->path.append(buf.path);
      op->operation = IOProxyDataOP::write;
      op->type = IO_PROXY_DATA;
      op->size = static_cast<size_t>(buf.buf.size);
      op->offset = buf.offset;
      op->ibuf = buf.buf.ptr;
      op->complete_callbacks.push_back([this, op]() {
        ioproxy_rpc_buffer ib(op->error_code);
        ib.msgpack_pack();
        write();
      });
      static_cast<IOProxy *>(hvs::HvsContext::get_context()->node)
          ->queue_op(op);
    } catch (exception &e) {
      // msg corrupt
      // pass
    }

    // send the resp back
    if (!resp.is_empty()) {
      write(resp.get_data());
    }
    // after work, such as close session
  }

  if (m_stop) {
    // remove from epoll
    UDT::epoll_remove_usock(parent->epoll_fd, socket_);
    UDT::close(socket_);
  }
}

void UDTSession::do_write() {
  if (m_stop) {
    return;
  }
  auto &item = write_queue_.front();
  // the data in item remains valid until the handler is called
  // since it will still be in the queue physically until then.
  RPCLIB_ASIO::async_write(
      socket_, clmdep_msgpack::buffer(item.data(), item.size()),
      write_strand_.wrap(
          [this, self](std::error_code ec, std::size_t transferred) {
            (void)transferred;
            if (!ec) {
              write_queue_.pop_front();
              if (write_queue_.size() > 0) {
                if (!exit_) {
                  do_write();
                }
              }
            } else {
              LOG_ERROR("Error while writing to socket: {}", ec);
            }

            if (exit_) {
              LOG_INFO("Closing socket");
              try {
                socket_.shutdown(RPCLIB_ASIO::ip::tcp::socket::shutdown_both);
              } catch (std::system_error &e) {
                (void)e;
                LOG_WARN(
                    "std::system_error during socket shutdown. "
                    "Code: {}. Message: {}",
                    e.code(), e.what());
              }
              socket_.close();
            }
          }));
}