#include "context.h"

namespace hvs {
HvsContext* HvsContext::_context_p = nullptr;
void init_context() {
  HvsContext* _context = HvsContext::get_context();
  _context->_log = init_logger();
  _context->_rest = init_rest();
}
void destroy_context() {
  HvsContext* _context = HvsContext::get_context();
  if (_context->_log != nullptr) {
    stop_log(_context->_log);
    stop_rest(_context->_rest);
    _context->_log = nullptr;
  }
}
}  // namespace hvs