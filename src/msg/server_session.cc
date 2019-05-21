#include "msg/server_session.h"
#include "context.h"
#include "io_proxy/rpc_bindings.hpp"
#include "io_proxy/rpc_types.h"
#include "msg/udt_server.h"

using namespace hvs;
using namespace std;
ServerSession::ServerSession(UDTServer *srv, UDTSOCKET socket)
    : parent(srv), unpacker(), socket_(socket), m_stop(false) {
  unpacker.reserve_buffer(10240000);
  writer = make_shared<UDTWriter>(socket_);
  writer->start();
}

void ServerSession::close() {
  m_stop = true;
  if(writer) {
    writer->close();
    writer.reset();
  }
  // should we close udt socket here ???
  //   write_strand_.post([this]() {
  //     socket_.close();
  //     parent_->close_session(shared_from_base<server_session>());
  //   });
}

// currently without asio, we use epoll.
// do_read would be called when epoll returned. session should copy buffers
// from UDT and return as quick as possible
void ServerSession::do_read() {
  constexpr std::size_t max_read_bytes = 1024000;
  constexpr std::size_t default_buffer_size = max_read_bytes;

  unsigned long rs = 0;
  //   UDT::getsockopt(socket_, 0, UDT_RCVDATA, &rcv_size, &var_size);
  if (UDT::ERROR ==
      (rs = UDT::recv(socket_, unpacker.buffer(), default_buffer_size, 0))) {
    dout(10) << "WARNING: recv error:" << UDT::getlasterror().getErrorMessage()
             << dendl;
    // maybe close session?
    m_stop = true;
    return;
  } else {
    unpacker.buffer_consumed(rs);
  }

  // function should return immediately, async op instead.
  // assemble the data op call
  clmdep_msgpack::unpacked result;
  while (unpacker.next(result) && !m_stop) {
    auto msg = result.get();

    // use this zone handle to keep memory live time
    // remember to pass it
    auto z = std::shared_ptr<RPCLIB_MSGPACK::zone>(result.zone().release());
    // handle the message
    try {
      auto buf = msg.as<ioproxy_rpc_buffer>();
      auto op = std::make_shared<IOProxyDataOP>();
      // TODO: use dynamic prefix path
      op->path.assign("/tmp/hvs/tests/data");
      op->path.append(buf.path);
      op->operation = IOProxyDataOP::write;
      op->type = IO_PROXY_DATA;
      op->size = static_cast<size_t>(buf.buf.size);
      op->offset = buf.offset;
      op->ibuf = buf.buf.ptr;
      op->id = buf.id;
      op->complete_callbacks.push_back([this, op, z]() {
        dout(-1) << " wait: " << chrono::duration_cast<std::chrono::microseconds>(op->op_submit - op->op_queued).count()
        << " processing: "<<chrono::duration_cast<std::chrono::microseconds>(op->op_complete - op->op_submit).count() << dendl;
        RPCLIB_MSGPACK::sbuffer data;
        ioproxy_rpc_buffer rb(op->error_code);
        rb.id = op->id;
        clmdep_msgpack::pack(data, rb);
        // move sem, zero copy
        // send the resp back
        writer->write(std::move(data));
        auto now = chrono::steady_clock::now();
          dout(-1) << " sending: " << chrono::duration_cast<std::chrono::microseconds>(now - op->op_complete).count()  << dendl;
      });
      static_cast<IOProxy *>(hvs::HvsContext::get_context()->node)
          ->queue_op(op);
        dout(-1) << "op-" << op->id << " queued on server" << dendl;
    } catch (exception &e) {
      // msg corrupt
      // pass
    }
    // after work, such as close session
  }

  if (m_stop) {
    // remove from epoll
    UDT::epoll_remove_usock(parent->epoll_fd, socket_);
    UDT::close(socket_);
  } else {
      if (unpacker.buffer_capacity() < max_read_bytes) {
          unpacker.reserve_buffer(max_read_bytes);
      }
  }
}
