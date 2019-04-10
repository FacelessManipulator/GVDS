//
// Created by miller on 2/26/19.
//
#pragma once

#ifndef HVS_CONTEXT
#define HVS_CONTEXT
#endif

#include <memory>
#include <string>

namespace hvs {
class RpcServer;
class Log;
class ConfigureSettings;
class HvsContext {
 public:
  static HvsContext* get_context() {
    if (_context_p == nullptr) _context_p = new HvsContext();
    return _context_p;
  }
  HvsContext() : _log(nullptr), _config(nullptr), _rpc(nullptr) {}

 public:
  std::string module_name;
  Log* _log;
  ConfigureSettings* _config;
  RpcServer* _rpc;

 private:
  static HvsContext* _context_p;
};

extern void init_context();
extern void destroy_context();
}  // namespace hvs

#include "config/ConfigureSettings.h"
#include "log/Log.h"
#include "common/debug.h"
#include "msg/rpc.h"