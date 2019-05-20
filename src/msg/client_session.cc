#include "msg/client_session.h"
#include "context.h"
#include "msg/udt_client.h"

using namespace hvs;
using namespace std;

ClientSession::ClientSession(UDTClient *srv, UDTSOCKET socket)
    : parent(srv), unpacker(), socket_(socket), m_stop(true) {
  unpacker.reserve_buffer(102400);
  seq_n = 0;
  writer = make_shared<UDTWriter>(socket_);
}

void ClientSession::start() {
    m_stop = false;
    writer->start();
    create("client session");
}

void *ClientSession::entry() {
    do_read();
}

void ClientSession::close() {
  m_stop = true;
  if (writer) {
    writer->close();
    writer.reset();
  }
  // should we close udt socket here ???
  //   write_strand_.post([this]() {
  //     socket_.close();
  //     parent_->close_session(shared_from_base<server_session>());
  //   });
}

int ClientSession::write(ioproxy_rpc_buffer &buffer) {
  clmdep_msgpack::sbuffer data;
  buffer.id = seq_n++;
  clmdep_msgpack::pack(data, buffer);
  // move sem, zero copy
  // send the resp back
  promise<void> ready_promise;

  shared_future<void> ready_future(ready_promise.get_future());
  futures[buffer.id] = ready_future;
  ready_promises[buffer.id] = move(ready_promise);
  writer->write(std::move(data));
  return buffer.id;
}

bool ClientSession::wait_op(int id) {
  auto it = futures.find(id);
  if (it == futures.end()) return true;
  shared_future<void> ft = it->second;
  // unlock
  ft.wait();
  // lock
  futures.erase(id);
  return true;
}

// currently without asio, we use epoll.
// do_read would be called when epoll returned. session should copy buffers
// from UDT and return as quick as possible
void ClientSession::do_read() {
  constexpr std::size_t max_read_bytes = 102400;
  constexpr std::size_t default_buffer_size = max_read_bytes;

  while (!m_stop) {
    unsigned long rs = 0;
    //   UDT::getsockopt(socket_, 0, UDT_RCVDATA, &rcv_size, &var_size);
    if (UDT::ERROR ==
        (rs = UDT::recv(socket_, unpacker.buffer(), default_buffer_size, 0))) {
      dout(10) << "WARNING: recv error:"
               << UDT::getlasterror().getErrorMessage() << dendl;
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
      //   auto z =
      //   std::shared_ptr<RPCLIB_MSGPACK::zone>(result.zone().release());
      // handle the message
      try {
        auto buf = msg.as<ioproxy_rpc_buffer>();
        auto it = ready_promises.find(buf.id);
        if(it == ready_promises.end()) {
          // error duplicated msg?
          continue;
        } else {
          promise<void> pm = std::move(it->second);
          ready_promises.erase(it);
          pm.set_value();

        }
      } catch (exception &e) {
        // msg corrupt
        // pass
      }
      // after work, such as close session
    }
  }
  if (m_stop) {
    // i will code next week
  }
}
