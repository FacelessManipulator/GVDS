/*
 * @Author: Hanjie,Zhou 
 * @Date: 2020-02-20 00:37:48 
 * @Last Modified by:   Hanjie,Zhou 
 * @Last Modified time: 2020-02-20 00:37:48 
 */
#pragma once
#include <stdint.h>

enum OpTypes {
  // filesystem operations
  statfs,

  // general operations
  getattr,
  rename,
  access,
  utimens,
  lock,

  // extend attributes
  setxattr,
  getxattr,
  listxattr,
  removexattr,

  // permission operations
  chmod,
  chown,

  // file operations
  truncate,
  open,
  read,
  write,
  flush,
  release,
  fsync,
  create,
  write_buf,
  read_buf,
  fallocate,
  flock,

  // directory operations
  mkdir,
  rmdir,
  opendir,
  readdir,
  releasedir,
  fsyncdir,

  // link operations
  readlink,
  unlink,
  symlink,
  link,

  // special
  bmap,
  ioctl,
  poll,
};

// hvs is GVDS's sub project name
namespace hvs {
class GVDSOperation {
  // the operation type
  uint8_t type;
  const uint32_t id;
};
}  // namespace hvs
