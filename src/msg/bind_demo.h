#pragma once
#include "msg/rpc.h"
#include "msg/stat_demo.h"

namespace hvs {
  inline void hvs_rpc_bind(RpcServer* rpc_server) {
    rpc_server->bind("1", [] { return 1; });
    rpc_server->bind("sleep_test", sleep);
    rpc_server->bind("stat_test", get_stat);
  }
}