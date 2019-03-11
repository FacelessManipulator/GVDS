//
// Created by miller on 2/26/19.
//
#pragma once

#include <memory>
#include <string>
namespace hvs {
class Log;
class HvsContext {
 public:
  static HvsContext* get_context() {
    if (_context_p == nullptr) _context_p = new HvsContext();
    return _context_p;
  }
  std::string module_name;
  Log* _log;

 private:
  static HvsContext* _context_p;
};
HvsContext* HvsContext::_context_p = nullptr;
}  // namespace hvs