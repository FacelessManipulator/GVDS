#pragma once

#include <stdint.h>
#include <boost/function.hpp>
#include <chrono>
#include <atomic>
#include <list>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <vector>

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
  uint64_t id; // not global unique
  int error_code;
  bool should_prepare;

  // callback invoked
  std::list<boost::function0<void>> complete_callbacks;

  // timepoint to track system perfomance
  using clock = std::chrono::steady_clock;
  clock::time_point op_queued;
  clock::time_point op_submit;
  clock::time_point op_complete;
  OP(): should_prepare(false), error_code(-1){}
};

struct IOProxyMetadataOP : public OP {
  enum Operation {
    stat,
    open,
    readdir,
    // TODO: finish the other metadata operations
  };
  Operation operation;
  const char* path;
  struct stat* buf; // return value pointer
  // readdir data
  std::vector<struct dirent> dirvector;
  IOProxyMetadataOP() :buf(nullptr) { should_prepare = true; }
  ~IOProxyMetadataOP() {
    if(buf)
      free(buf);
    buf = nullptr;
  }
};

struct IOProxyDataOP : public OP {
  enum Operation {
    read,
    write,
    // TODO: finish the other metadata operations
  };
  Operation operation;
  const char* path;
  char* obuf; // mutable buffer for read
  const char* ibuf; // imutable buffer for write
  size_t size;
  off_t  offset;
  ~IOProxyDataOP() {
    // we don't manage write buf
    if(obuf)
      free(obuf);
    obuf = nullptr;
  }
};
}  // namespace hvs