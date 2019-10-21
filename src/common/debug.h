#pragma once

/* Global version of the stuff in common/dout.h
 */
// #include "context.h"
#ifndef HVS_CONTEXT
#error "include context.h before include debug.h"
#endif
#include "log/Entry.h"
#include "log/Log.h"

#define dout(v)                                          \
  do {                                                   \
    auto _cct = hvs::HvsContext::get_context();          \
    hvs::EntryPtr _dout_e = _cct->_log->create_entry(v, __FUNCTION__); \
    std::ostream *_dout = &_dout_e->get_ostream();       \
  *_dout

#define dendl_impl                   \
  std::flush;                        \
  _cct->_log->submit_entry(_dout_e); \
  }                                  \
  while (0)

#define dendl dendl_impl