#pragma once

#include <stdint.h>
#include <sys/stat.h>
#include "rpc/server.h"
#include <string>

namespace gvds {
struct stat_buffer {
  int error_code;
  unsigned st_dev;
  unsigned st_ino;
  unsigned st_mode;
  unsigned st_nlink;
  unsigned st_uid;
  unsigned st_gid;
  unsigned st_size;
  unsigned st_atim;
  unsigned st_mtim;
  unsigned st_ctim;
  stat_buffer(struct stat* st) {
    st_dev = st->st_dev;    
    st_ino = st->st_ino;
    st_mode = st->st_mode;
    st_nlink = st->st_nlink;
    st_uid = st->st_uid;
    st_gid = st->st_gid;
    st_size = st->st_size;
    st_atim = st->st_atim.tv_sec;
    st_mtim = st->st_mtim.tv_sec;
    st_ctim = st->st_ctim.tv_sec;
    error_code = 0;
  }
  stat_buffer(int i): error_code(i){}
  stat_buffer():error_code(-1) {}
  MSGPACK_DEFINE_ARRAY(st_dev, st_ino, st_mode, st_nlink, st_uid, st_gid,
                       st_size, st_atim, st_mtim, st_ctim);
};

// this function should only be used in test!
inline stat_buffer get_stat(std::string pathname) {
  struct stat _st;
  int err;
  if ((err = stat(pathname.c_str(), &_st)) == 0) {
    return stat_buffer(&_st);
  } else {
    return stat_buffer(err);
  }
}
}
