//
// Created by yaowen on 4/29/19.
// 北航系统结构所-存储组
//

#ifndef FUSE_USE_VERSION
#define FUSE_USE_VERSION 31
#endif
#include <dirent.h>
#include <error.h>
#include <fuse3/fuse.h>
#include <fuse3/fuse_lowlevel.h>
#include <getopt.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctime>
#include <iostream>
#include <vector>
#include "client/client.h"
#include "client/fuse_mod.h"
#include "client/graph_mod.h"
#include "client/msg_mod.h"
#include "client/queue.h"
#include "client/zone_mod.h"
#include "context.h"
#include "hvs_struct.h"
#include "io_proxy/rpc_types.h"

extern bool zonechecker_run;
#define FUSE_DEBUG_LEVEL 25
#define HVS_FUSE_DATA \
  ((struct ::hvs::ClientFuseData *)fuse_get_context()->private_data)

using namespace std;

namespace hvs {
// TODO: 工具函数，分割字符串
std::vector<std::string> splitWithStl(const std::string str,
                                      const std::string pattern) {
  std::vector<std::string> resVec;

  if ("" == str) {
    return resVec;
  }
  //方便截取最后一段数据
  std::string strs = str + pattern;

  size_t pos = strs.find(pattern);
  size_t size = strs.size();

  while (pos != std::string::npos) {
    std::string x = strs.substr(0, pos);
    resVec.push_back(x);
    strs = strs.substr(pos + 1, size);
    pos = strs.find(pattern);
  }
  return resVec;
}

void *hvsfs_init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
  cfg->ac_attr_timeout_set = 1;
  cfg->attr_timeout = 1;
  cfg->entry_timeout = 1;
  cfg->negative_timeout = 1;
  conn->max_readahead = 524288;
  conn->max_read = 524288;
  return HVS_FUSE_DATA;
}

// stat
int hvsfs_getattr(const char *path, struct stat *stbuf,
                  struct fuse_file_info *fi) {
  auto zs_res = HVS_FUSE_DATA->client->zone->isZoneSpacePath(path);
  // if path is not remote file
  if (zs_res < 0) {
    return zs_res;
  } else if (zs_res < 2) {
    return HVS_FUSE_DATA->client->zone->getattr(path, stbuf);
  }

  auto [iop, rpath] = HVS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    //return -ENETUNREACH;
    return -EPERM; // 当找不到远程路径时，说明当前的空间为不存在，返回无权限；
  }

  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  auto res = HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_stat", rpath);
  if (!res.get()) {
    // timeout exception raised
    return -ENOENT;
  }

  auto retbuf = res->as<ioproxy_rpc_statbuffer>();
  if (retbuf.error_code) {
    // stat failed on remote server
    return retbuf.error_code;
  }

  memset(stbuf, 0, sizeof(struct stat));
  stbuf->st_dev = static_cast<__dev_t>(retbuf.st_dev);
  stbuf->st_ino = static_cast<__ino_t>(retbuf.st_ino);
  stbuf->st_mode = static_cast<__mode_t>(retbuf.st_mode);
  stbuf->st_nlink = static_cast<__nlink_t>(retbuf.st_nlink);
  stbuf->st_uid = static_cast<__uid_t>(retbuf.st_uid);
  stbuf->st_gid = static_cast<__gid_t>(retbuf.st_gid);
  stbuf->st_rdev = static_cast<__dev_t>(retbuf.st_rdev);
  stbuf->st_size = retbuf.st_size;
  stbuf->st_atim.tv_nsec = retbuf.st_atim_tv_nsec;
  stbuf->st_atim.tv_sec = retbuf.st_atim_tv_sec;
  stbuf->st_mtim.tv_nsec = retbuf.st_mtim_tv_nsec;
  stbuf->st_mtim.tv_sec = retbuf.st_mtim_tv_sec;
  stbuf->st_ctim.tv_nsec = retbuf.st_ctim_tv_nsec;
  stbuf->st_ctim.tv_sec = retbuf.st_ctim_tv_sec;
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return retbuf.error_code;
}

int hvsfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                  off_t offset, struct fuse_file_info *fi,
                  enum fuse_readdir_flags flags) {
  auto zs_res = HVS_FUSE_DATA->client->zone->isZoneSpacePath(path);
  if (zs_res < 0) {
    return zs_res;
  } else if (zs_res < 2) {
    return HVS_FUSE_DATA->client->zone->readdir(path, buf, filler);
  }

  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  auto [iop, rpath] = HVS_FUSE_DATA->client->graph->get_mapping(path);

  // not exists
  if (!iop) {
    return -ENOENT;
  }

  auto res = HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_readdir", rpath);
  if (!res.get()) {
    // timeout exception raised
    return -ENOENT;
  }

  int retstat = 0;
  auto ents = res->as<std::vector<ioproxy_rpc_statbuffer>>();
  for (const ioproxy_rpc_statbuffer &ent : ents) {
    struct stat dirfilestat;
    memset(&dirfilestat, 0, sizeof(struct stat));
    dirfilestat.st_dev = static_cast<__dev_t>(ent.st_dev);
    dirfilestat.st_ino = static_cast<__ino_t>(ent.st_ino);
    dirfilestat.st_mode = static_cast<__mode_t>(ent.st_mode);
    dirfilestat.st_nlink = static_cast<__nlink_t>(ent.st_nlink);
    dirfilestat.st_uid = static_cast<__uid_t>(ent.st_uid);
    dirfilestat.st_gid = static_cast<__gid_t>(ent.st_gid);
    dirfilestat.st_rdev = static_cast<__dev_t>(ent.st_rdev);
    dirfilestat.st_size = ent.st_size;
    dirfilestat.st_atim.tv_nsec = ent.st_atim_tv_nsec;
    dirfilestat.st_atim.tv_sec = ent.st_atim_tv_sec;
    dirfilestat.st_mtim.tv_nsec = ent.st_mtim_tv_nsec;
    dirfilestat.st_mtim.tv_sec = ent.st_mtim_tv_sec;
    dirfilestat.st_ctim.tv_nsec = ent.st_ctim_tv_nsec;
    dirfilestat.st_ctim.tv_sec = ent.st_ctim_tv_sec;
    if (filler(buf, ent.d_name.c_str(), &dirfilestat, 0,
               static_cast<fuse_fill_dir_flags>(0)) != 0) {
      return -ENOMEM;
    }
    retstat = ent.error_code;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;

  return retstat;
}

int hvsfs_open(const char *path, struct fuse_file_info *fi) {
  int retstat = 0;
  int zs_res = HVS_FUSE_DATA->client->zone->isZoneSpacePath(path);
  if (zs_res < 0) {
    return zs_res;
  } else if (zs_res < 2) {
    // zs_res < 0 or zs_res >0 both means zone mod handle this operation
    return -ENOTSUP;
  }
  auto [iop, rpath] = HVS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  auto res = HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_open",
                                              rpath.c_str(), fi->flags);
  if (!res.get()) {
    // timeout exception raised
    return -ENOENT;
  }
  retstat = res->as<int>();
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  if (retstat > 0) {
    fi->fh = retstat;
    return 0;
  } else
    return retstat;
}

int hvsfs_read(const char *path, char *buf, size_t size, off_t offset,
               struct fuse_file_info *fi) {
  auto [iop, rpath] = HVS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }

  ioproxy_rpc_buffer _buffer(rpath.c_str(), offset, size);
  _buffer.fid = fi->fh;
  _buffer.flags = fi->flags;

  if (HVS_FUSE_DATA->fuse_client->use_udt) {
    // UDT version
    auto res = HVS_FUSE_DATA->client->rpc->read_data(iop, _buffer);
    if (!res) {
      return -ETIMEDOUT;
    }
    if (res->error_code < 0) {
      // stat failed on remote server
      return res->error_code;
    }
    memcpy(buf, res->buf.ptr, res->buf.size);
    return res->buf.size;
  } else {
    auto res = HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_read", rpath,
                                                size, offset, fi->fh);
    if (!res.get()) {
      // timeout exception raised
      return -ENOENT;
    }
    auto retbuf = res->as<ioproxy_rpc_buffer>();
    if (retbuf.error_code != 0) {
      // stat failed on remote server
      return -retbuf.error_code;
    }
    memcpy(buf, retbuf.buf.ptr, retbuf.buf.size);
    return retbuf.buf.size;
  }
}

int hvsfs_write(const char *path, const char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi) {
  auto [iop, rpath] = HVS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }

  ioproxy_rpc_buffer _buffer(rpath.c_str(), buf, offset, size);
  _buffer.is_read = false;
  _buffer.fid = fi->fh;
  _buffer.flags = fi->flags;

  if (HVS_FUSE_DATA->fuse_client->async_mode) {
    auto buf2q = std::make_shared<hvs::Buffer>(rpath, buf, offset, size);
    buf2q->dest = iop;
    HVS_FUSE_DATA->client->queue->queue_buffer(buf2q);
    return size;
  } else {
    // tcp version
    auto res = HVS_FUSE_DATA->client->rpc->call(
        iop, "ioproxy_write", rpath.c_str(), _buffer, size, offset);
    if (!res.get()) {
      // timeout exception raised
      return -ENOENT;
    }
    auto retbuf = res->as<int>();
    return retbuf;
  }
  // write may failed on remote server
}

int hvsfs_access(const char *path, int mode) {
  int zs_res = HVS_FUSE_DATA->client->zone->isZoneSpacePath(path);
  if (zs_res < 0) {
    return zs_res;
  } else if (zs_res < 2) {
    return 0;
  }
  auto [iop, rpath] = HVS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  auto res = HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_access",
                                              rpath.c_str(), mode);
  if (!res.get()) {
    // timeout exception raised
    return -ENOENT;
  }
  auto retstat = res->as<int>();
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return retstat;
}

int hvsfs_opendir(const char *path, struct fuse_file_info *fi) {
  int retstat = 0;

  return retstat;
}

void hvsfs_destroy(void *private_data) { HVS_FUSE_DATA->client->stop(); }

int hvsfs_truncate(const char *path, off_t offset, struct fuse_file_info *fi) {
  auto [iop, rpath] = HVS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }

  auto res = HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_truncate",
                                              rpath.c_str(), offset);
  if (!res.get()) {
    // timeout exception raised
    return -ENOENT;
  }
  auto retstat = res->as<int>();
  return retstat;
}

int hvsfs_readlink(const char *path, char *link, size_t size) {
  auto [iop, rpath] = HVS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }

  auto res =
      HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_readlink", rpath, size);
  if (!res.get()) {
    // timeout exception raised
    return -ENOENT;
  }

  auto retbuf = res->as<std::string>();
  memcpy(link, retbuf.c_str(), retbuf.size());
  return retbuf.size();
}

int hvsfs_mknod(const char *path, mode_t mode, dev_t dev) {
  int retstat = 0;

  return retstat;
}
int hvsfs_mkdir(const char *path, mode_t mode) {
  std::vector<std::string> namev = splitWithStl(path, "/");
  int nvsize = static_cast<int>(namev.size());
  if (namev.size() <= 3) {
    return -EPERM;
  }
  auto [iop, rpath] = HVS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }

  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  auto res = HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_mkdir",
                                              rpath.c_str(), mode);
  if (!res.get()) {
    // timeout exception raised
    return -ENOENT;
  }
  auto retstat = res->as<int>();
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return retstat;
}

int hvsfs_unlink(const char *path) {
  auto [iop, rpath] = HVS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }

  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  auto res = HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_unlink", rpath);
  if (!res.get()) {
    // timeout exception raised
    return -ENOENT;
  }
  auto retstat = res->as<int>();
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return retstat;
}

int hvsfs_rmdir(const char *path) {
  //跳过本地
  std::vector<std::string> namev = splitWithStl(path, "/");
  int nvsize = static_cast<int>(namev.size());
  if (namev.size() <= 3) {
    return -EPERM;
  }
  // 删除远程
  auto [iop, rpath] = HVS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }

  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  auto res = HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_rmdir", rpath);
  if (!res.get()) {
    // timeout exception raised
    return -ENOENT;
  }
  auto retstat = res->as<int>();
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return retstat;
}

int hvsfs_symlink(const char *path, const char *newpath) {
  int retstat = 0;
  std::cout << "TODO: 软连接操作有问题，待修复！" << std::endl;
  //        std::string root("/");
  //        std::string paths(path);
  //        std::string fullpath = root+paths;
  //        auto ip = new std::string(ioproxy_ip.c_str());
  //        auto port =
  //        hvs::HvsContext::get_context()->_config->get<int>("rpc.port");
  //        RpcClient client(*ip, static_cast<const unsigned int>(*port));
  //        auto res = client.call("ioproxy_symlink", fullpath.c_str(),
  //        newpath); retstat = res->as<int>();
  return retstat;
}

int hvsfs_rename(const char *path, const char *newpath, unsigned int flags) {
  //跳过本地
  std::vector<std::string> namev = splitWithStl(path, "/");
  int nvsize = static_cast<int>(namev.size());
  if (namev.size() <= 3) {
    return -EPERM;
  }

  // access content level
  auto [siop, srpath] = HVS_FUSE_DATA->client->graph->get_mapping(path);
  auto [diop, drpath] = HVS_FUSE_DATA->client->graph->get_mapping(newpath);
  // not exists
  if (!siop || !diop) {
    return -ENOENT;
  } else if (siop->uuid != diop->uuid) {
    // over ioproxy move is not support yet
    return -ENOTSUP;
  }

  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  auto res =
      HVS_FUSE_DATA->client->rpc->call(siop, "ioproxy_rename", srpath, drpath);
  if (!res.get()) {
    // timeout exception raised
    return -ENOENT;
  }
  auto retstat = res->as<int>();
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return retstat;
}

int hvsfs_link(const char *path, const char *newpath) {
  int retstat = 0;
  std::cout << "TODO: 硬连接操作有问题，待修复！" << std::endl;
  //        auto ip = new std::string(ioproxy_ip.c_str());
  //        auto port =
  //        hvs::HvsContext::get_context()->_config->get<int>("rpc.port");
  //        RpcClient client(*ip, static_cast<const unsigned int>(*port));
  //        auto res = client.call("ioproxy_link", path, newpath);
  //        retstat = res->as<int>();
  return retstat;
}

int hvsfs_chmod(const char *path, mode_t mode, struct fuse_file_info *fi) {
  std::vector<std::string> namev = splitWithStl(path, "/");
  int nvsize = static_cast<int>(namev.size());
  if (namev.size() <= 3) {
    return -EPERM;
  }
  auto [iop, rpath] = HVS_FUSE_DATA->client->graph->get_mapping(path);

  // not exists
  if (!iop) {
    return -ENOENT;
  }

  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  auto res =
      HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_chmod", rpath, mode);
  if (!res.get()) {
    // timeout exception raised
    return -ENOENT;
  }
  auto retstat = res->as<int>();
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return retstat;
}

int hvsfs_chown(const char *path, uid_t uid, gid_t gid,
                struct fuse_file_info *fi) {
  std::vector<std::string> namev = splitWithStl(path, "/");
  int nvsize = static_cast<int>(namev.size());
  if (namev.size() <= 3) {
    return -EPERM;
  }
  auto [iop, rpath] = HVS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }

  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  auto res =
      HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_chown", rpath, uid, gid);
  if (!res.get()) {
    // timeout exception raised
    return -ENOENT;
  }
  auto retstat = res->as<int>();
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return retstat;
}

int hvsfs_statfs(const char *, struct statvfs *) {
  int retstat = 0;

  return retstat;
}

int hvsfs_flush(const char *, struct fuse_file_info *) {
  int retstat = 0;

  return retstat;
}

int hvsfs_release(const char *, struct fuse_file_info *) {
  int retstat = 0;
  return retstat;
}

int hvsfs_fsync(const char *, int, struct fuse_file_info *) {
  int retstat = 0;

  return retstat;
}

int hvsfs_releasedir(const char *, struct fuse_file_info *) {
  int retstat = 0;

  return retstat;
}

int hvsfs_create(const char *path, mode_t mode, struct fuse_file_info *) {
  std::vector<std::string> namev = splitWithStl(path, "/");
  int nvsize = static_cast<int>(namev.size());
  if (namev.size() <= 3) {
    return -EPERM;
  }
  auto [iop, rpath] = HVS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }

  dout(FUSE_DEBUG_LEVEL) << "req-" << path << dendl;
  auto res =
      HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_create", rpath, mode);
  if (!res.get()) {
    // timeout exception raised
    return -ENOENT;
  }
  auto retstat = res->as<int>();
  dout(FUSE_DEBUG_LEVEL) << "remote finish req: " << path << dendl;
  return retstat;
}

int hvsfs_utimens(const char *path, const struct timespec tv[2],
                  struct fuse_file_info *fi) {
  std::vector<std::string> namev = splitWithStl(path, "/");
  int nvsize = static_cast<int>(namev.size());
  if (namev.size() <= 3) {
    return -EPERM;
  }
  auto [iop, rpath] = HVS_FUSE_DATA->client->graph->get_mapping(path);
  // access content level
  long int sec0n = tv[0].tv_nsec;
  long int sec0s = tv[0].tv_sec;
  long int sec1n = tv[1].tv_nsec;
  long int sec1s = tv[1].tv_sec;
  // not exists
  if (!iop) {
    return -ENOENT;
  }

  auto res = HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_utimes", rpath,
                                              sec0n, sec0s, sec1n, sec1s);
  if (!res.get()) {
    // timeout exception raised
    return -ENOENT;
  }
  auto retstat = res->as<int>();
  return retstat;
}

struct fuse_operations hvsfs_oper = {
    .getattr = hvsfs_getattr,
    .readlink = hvsfs_readlink,
    .mknod = hvsfs_mknod,  // TODO: create函数重复，暂时不做
    .mkdir = hvsfs_mkdir,
    .unlink = hvsfs_unlink,
    .rmdir = hvsfs_rmdir,
    .symlink = hvsfs_symlink,  // TODO: 软连接操作暂时有问题，待修复；
    .rename = hvsfs_rename,
    .link = hvsfs_link,  // TODO: 硬连接操作暂时有问题，待修复；
    .chmod =
        hvsfs_chmod,  // TODO: 虚拟数据空间相关，涉及到权限，之后需要统一修改；
    .chown =
        hvsfs_chown,  // TODO: 虚拟数据空间相关，涉及到权限，之后需要统一修改；
    .truncate = hvsfs_truncate,
    .open = hvsfs_open,
    .read = hvsfs_read,
    .write = hvsfs_write,
    .statfs = hvsfs_statfs,    // TODO: vfs 相关，暂时不做
    .flush = hvsfs_flush,      // TODO: cache 相关，暂时不做
    .release = hvsfs_release,  // TODO: 未保存文件fd, 暂时不做
    .fsync = hvsfs_fsync,      // TODO: caced 相关暂时不做
    .setxattr = NULL,
    .getxattr = NULL,
    .listxattr = NULL,
    .removexattr = NULL,
    .opendir = hvsfs_opendir,
    .readdir = hvsfs_readdir,
    .releasedir = hvsfs_releasedir,  // TODO: 未保存DIR, 暂时不做
    .fsyncdir = NULL,
    .init = hvsfs_init,
    .destroy = hvsfs_destroy,
    .access = hvsfs_access,
    .create = hvsfs_create,
    .lock = NULL,
    .utimens = hvsfs_utimens,
    .bmap = NULL,
    .ioctl = NULL,
    .poll = NULL,
    .write_buf = NULL,
    .read_buf = NULL,
    .flock = NULL,
    .fallocate = NULL,
};
}  // namespace hvs
