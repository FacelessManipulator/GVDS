#include "gvds_context.h"

namespace gvds {
HvsContext* HvsContext::_context_p = nullptr;
void init_context(const std::string& config_path) {
  HvsContext* _context = HvsContext::get_context();
  // TODO: use command line parameter config_path
  _context->_config = init_config(config_path);
  if (!_context->_config) {
    // failed to start config component
    std::cerr << "ERROR: cannot read config file." << std::endl << std::flush;
    exit(-1);
  }
  _context->_log = init_logger();
  if (!_context->_log) {
    // failed to start log component
    std::cerr << "ERROR: cannot write log file." << std::endl << std::flush;
    exit(-1);
  }
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
}  // namespace gvds