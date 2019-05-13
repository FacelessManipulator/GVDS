#include "context.h"

namespace hvs {
HvsContext* HvsContext::_context_p = nullptr;
void init_context() {
  HvsContext* _context = HvsContext::get_context();
  // TODO: use command line parameter config_path
  _context->_config = init_config("/tmp/hvs/tests/data/example.cfg");
  if (!_context->_config) {
    // failed to start config component
    exit(-1);
  }
  _context->_log = init_logger();
  if (!_context->_log) {
    // failed to start log component
    exit(-1);
  }
  _context->ioproxy_rootdir = "/tmp/hvs/tests/data";
}
void destroy_context() {
  HvsContext* _context = HvsContext::get_context();
  if (_context->_log != nullptr) {
    stop_log(_context->_log);
    _context->_log = nullptr;
  }
  if (_context->_config != nullptr) {
    delete _context->_config;
    _context->_config = nullptr;
  }
}
}  // namespace hvs