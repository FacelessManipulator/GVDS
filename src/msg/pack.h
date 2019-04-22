#pragma once

// a set of helper functions based on rpclib msgpack
#include "rpc/msgpack.hpp"

namespace hvs{
  template <typename T>
  inline std::shared_ptr<clmdep_msgpack::sbuffer> pack(T& obj) {
    auto buffer = std::make_shared<clmdep_msgpack::sbuffer>();
    clmdep_msgpack::pack(*buffer, obj);
    return buffer;
  }
  template <typename T>
  inline T unpack(std::shared_ptr<clmdep_msgpack::sbuffer> buffer) {
    auto obj = clmdep_msgpack::unpack(buffer->data(), buffer->size());
    return obj.as<T>();
  }
}