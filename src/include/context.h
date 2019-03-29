//
// Created by miller on 2/26/19.
//
#pragma once

#include <memory>
#include <string>
#include "log/Log.h"
#include "common/RestServer.h"
class RestServer;

namespace hvs {
class HvsContext {
 public:
  static HvsContext* get_context() {
    if (_context_p == nullptr) _context_p = new HvsContext();
    return _context_p;
  }
  std::string module_name;
  Log* _log;
  RestServer* _rest;

 private:
  static HvsContext* _context_p;
};

extern void init_context();
extern void destroy_context();
}  // namespace hvs