//
// Created by yaowen on 5/4/19.
// 北航系统结构所-存储组
//

#ifndef HVSONE_RPC_TYPES_H
#define HVSONE_RPC_TYPES_H
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "common/buffer.h"

namespace hvs {
struct ioproxy_rpc_statbuffer {
  int error_code;
  int st_dev;
  int st_ino;
  int st_mode;
  int st_nlink;
  int st_uid;
  int st_gid;
  int st_rdev;
  int st_size;
  int st_blksize;
  int st_blocks;
  int st_atim_tv_nsec;
  int st_atim_tv_sec;
  int st_mtim_tv_nsec;
  int st_mtim_tv_sec;
  int st_ctim_tv_nsec;
  int st_ctim_tv_sec;
  ioproxy_rpc_statbuffer(struct stat* st) {
    error_code = 0;
    st_dev = static_cast<int>(st->st_dev);
    st_ino = static_cast<int>(st->st_ino);
    st_mode = st->st_mode;
    st_nlink = static_cast<int>(st->st_nlink);
    st_uid = st->st_uid;
    st_gid = st->st_gid;
    st_rdev = static_cast<int>(st->st_rdev);
    st_size = static_cast<int>(st->st_size);
    st_blksize = static_cast<int>(st->st_blksize);
    st_blocks = static_cast<int>(st->st_blocks);
    st_atim_tv_nsec = static_cast<int>(st->st_atim.tv_nsec);
    st_atim_tv_sec = static_cast<int>(st->st_atim.tv_sec);
    st_mtim_tv_nsec = static_cast<int>(st->st_mtim.tv_nsec);
    st_mtim_tv_sec = static_cast<int>(st->st_mtim.tv_sec);
    st_ctim_tv_nsec = static_cast<int>(st->st_ctim.tv_nsec);
    st_ctim_tv_sec = static_cast<int>(st->st_ctim.tv_sec);
  }
  ioproxy_rpc_statbuffer() : error_code(-1) {}
  ioproxy_rpc_statbuffer(int i)
      : error_code(i) { /*当反回值error_code为小于0值的时候，表示产生了错误*/
  }
  MSGPACK_DEFINE_ARRAY(error_code, st_dev, st_ino, st_mode, st_nlink, st_uid,
                       st_gid, st_rdev, st_size, st_blksize, st_blocks,
                       st_atim_tv_nsec, st_atim_tv_sec, st_mtim_tv_nsec,
                       st_mtim_tv_sec, st_ctim_tv_nsec, st_ctim_tv_sec)
};

struct ioproxy_rpc_buffer {
  int error_code;
  bool is_read;
  unsigned long id;
  clmdep_msgpack::type::raw_ref buf;
  unsigned long offset;
  unsigned long read_size;
  std::string path;
  bool finalize_buf;
  int fid;
  int flags;
  // for write request purpose
  ioproxy_rpc_buffer(const char* _path, const char* buffer, unsigned long off,
                     int _size)
      : offset(off), path(_path), is_read(false), finalize_buf(false) {
    error_code = 0;
    buf.ptr = buffer;
    buf.size = _size;
  }
  // for read request purpose
  ioproxy_rpc_buffer(const char* _path, unsigned long off, int _size)
      : offset(off), path(_path), is_read(true), finalize_buf(false) {
    error_code = 0;
    read_size = _size;
  }

  ioproxy_rpc_buffer() : error_code(-1), is_read(true), finalize_buf(false) {}
  ioproxy_rpc_buffer(int i)
      : error_code(i), is_read(true), finalize_buf(false) {
    /*当反回值error_code为小于0值的时候，表示产生了错误*/
  }
  ioproxy_rpc_buffer(const ioproxy_rpc_buffer& oths) {
    // we treat left value copy as move semantic to support rpc
//    ioproxy_rpc_buffer(std::move(oths));

      this->error_code = oths.error_code;
      this->is_read = oths.is_read;
      this->id = oths.id;
      this->buf = oths.buf;
      this->offset = oths.offset;
      this->read_size = oths.read_size;
      this->path = oths.path;
      this->finalize_buf = oths.finalize_buf;
      this->fid = oths.fid;
      this->flags = oths.flags;
      // we have to provide a const function to rpclib
      // and we also have to modify oths ptr to move ownership of memory
      // const_cast is not a good chioce but pass the compile
      auto oths_nonconst = const_cast<ioproxy_rpc_buffer*>(&oths);
      oths_nonconst->buf.ptr = nullptr;
      oths_nonconst->buf.size = 0;
      oths_nonconst->finalize_buf = false;
  };
  ioproxy_rpc_buffer(Buffer& oths) {
    // we treat left value copy as move semantic to support rpc
//    ioproxy_rpc_buffer(std::move(oths));
      this->is_read = false;
      this->buf = oths.buf;
      this->offset = oths.offset;
      this->read_size = 0;
      this->path = oths.path;
      this->finalize_buf = true;
      this->flags = 0;
      // we have to provide a const function to rpclib
      // and we also have to modify oths ptr to move ownership of memory
      // const_cast is not a good chioce but pass the compile
      oths.buf.ptr = nullptr;
      oths.buf.size = 0;
  };
  ioproxy_rpc_buffer(const ioproxy_rpc_buffer&& oths) {
    this->error_code = oths.error_code;
    this->is_read = oths.is_read;
    this->id = oths.id;
    this->buf = oths.buf;
    this->offset = oths.offset;
    this->read_size = oths.read_size;
    this->path = oths.path;
    this->finalize_buf = oths.finalize_buf;
      this->fid = oths.fid;
      this->flags = oths.flags;
    // we have to provide a const function to rpclib
    // and we also have to modify oths ptr to move ownership of memory
    // const_cast is not a good chioce but pass the compile
    auto oths_nonconst = const_cast<ioproxy_rpc_buffer*>(&oths);
    oths_nonconst->buf.ptr = nullptr;
    oths_nonconst->buf.size = 0;
    oths_nonconst->finalize_buf = false;
  }
  ~ioproxy_rpc_buffer() {
    if (finalize_buf && buf.ptr) {
      free((void*)buf.ptr);
      buf.ptr = nullptr;
      buf.size = 0;
    }
  }
  MSGPACK_DEFINE_ARRAY(error_code, path, buf, offset, is_read, id, read_size, fid, flags);
};

struct ioproxy_rpc_dirent {
  int error_code;
  int d_ino;
  int d_off;
  unsigned short d_reclen;
  unsigned char dtype;
  std::string d_name;
  ioproxy_rpc_dirent(struct dirent* ent) {
    error_code = 0;
    d_ino = static_cast<int>(ent->d_ino);
    d_off = static_cast<int>(ent->d_off);
    d_reclen = ent->d_reclen;
    dtype = ent->d_type;
    d_name = ent->d_name;
  }
  ioproxy_rpc_dirent() : error_code(-1) {}
  ioproxy_rpc_dirent(int i)
      : error_code(i) { /*当反回值error_code为小于0值的时候，表示产生了错误*/
  }
  MSGPACK_DEFINE_ARRAY(error_code, d_ino, d_off, d_reclen, dtype, d_name);
};
}  // namespace hvs

#endif  // HVSONE_RPC_TYPES_H
