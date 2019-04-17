#pragma once

#include <stdint.h>
#include <boost/function.hpp>
#include <chrono>
#include <atomic>
#include <list>

// generic op class
namespace hvs {

enum OPType {
  GENERIC,
  IO_PROXY_CONTROLL,
  IO_PROXY_METADATA,
  IO_PROXY_DATA,
};

struct OP {
  OPType type;
  uint64_t id;  // not global unique

  // callback invoked
  std::list<boost::function0<void>> complete_callbacks;

  // timepoint to track system perfomance
  using clock = std::chrono::steady_clock;
  clock::time_point op_queued;
  clock::time_point op_submit;
  clock::time_point op_complete;
};

struct IOProxyMetadataOP : public OP {
  enum Operation {
    stat,
    open,
    // TODO: finish the other metadata operations
  };
  Operation operation;
  const char* path;
  void* retval; // return value pointer
};
}  // namespace hvs