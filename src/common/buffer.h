#pragma once
#include <functional>
#include "rpc/client.h"
#include "gvds_struct.h"

namespace gvds {
struct Buffer {
  clmdep_msgpack::type::raw_ref buf;
  uint64_t offset;
  std::string path;
  std::shared_ptr<IOProxyNode> dest;
  std::function<void()> callback;
  // prevent error write
  Buffer() : offset(0), path("/dev/null") {}
  Buffer(const std::string& p, const char* b, uint64_t off, unsigned long size) : path(p), offset(off) {
    // free at the end of queue
    buf.ptr = (const char*)std::malloc(size);
    buf.size = size;
    std::memcpy((void *)buf.ptr, b, size);
  }
  Buffer(Buffer&& oths) {
    buf.ptr = oths.buf.ptr;
    buf.size = oths.buf.size;
    oths.buf.ptr = nullptr;
    oths.buf.size = 0;
    path = std::move(oths.path);
    offset = oths.offset;
  }
  Buffer(const Buffer& oths) {
    buf.ptr = oths.buf.ptr;
    buf.size = oths.buf.size;
    path = oths.path;
    offset = oths.offset;
  };
  ~Buffer() {
    if (buf.ptr) {
    // delete buffer in clear state
    // delete buf.ptr;
      buf.ptr = nullptr;
      buf.size = 0;
    }
  }
  void append(clmdep_msgpack::type::raw_ref buf_app) {
    // TODO: these lines of code would be very dangerous without concerning of error malloc/realloc
    buf.ptr = (const char *)std::realloc((void *)(buf.ptr), buf.size + buf_app.size);
    memcpy((void *)(buf.ptr + buf.size), buf_app.ptr, buf_app.size);
    buf.size += buf_app.size;
    // NOTICE: buf_app is not freed
  }
  void destroy() {
    if (buf.ptr) {
      // delete buffer in clear state
      std::free((void *)buf.ptr);
      buf.ptr = nullptr;
      buf.size = 0;
    }
  }
  MSGPACK_DEFINE_ARRAY(path, buf, offset);
};
}  // namespace