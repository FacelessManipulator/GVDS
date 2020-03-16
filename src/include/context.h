//
// Created by miller on 2/26/19.
//
#pragma once

#ifndef GVDS_CONTEXT
#define GVDS_CONTEXT
#endif

#include <memory>
#include <string>

namespace gvds {
class Log;
class ConfigureSettings;
class IOProxy;
struct Node;
class IOProxy;
class HvsContext {
 public:
  static HvsContext* get_context() {
    if (_context_p == nullptr) _context_p = new HvsContext();
    return _context_p;
  }
  HvsContext()
      : _log(nullptr), _config(nullptr), node(nullptr) {}

 public:
  Log* _log;
  ConfigureSettings* _config;
  Node* node;

 private:
  static HvsContext* _context_p;
};

extern void init_context(const std::string& config_path = "");
extern void destroy_context();
}  // namespace gvds

#include "common/debug.h"
#include "config/ConfigureSettings.h"
#include "log/Log.h"
#include "msg/node.h"