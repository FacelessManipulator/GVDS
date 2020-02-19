/*
 * @Author: Hanjie,Zhou
 * @Date: 2020-02-20 00:36:32
 * @Last Modified by:   Hanjie,Zhou
 * @Last Modified time: 2020-02-20 00:36:32
 */
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
#include <experimental/filesystem>
#include <iostream>
#include <vector>
#include "client/cache_mod.h"
#include "client/client.h"
#include "client/fuse_mod.h"
#include "client/graph_mod.h"
#include "client/msg_mod.h"
#include "client/queue.h"
#include "client/readahead.h"
#include "client/zone_mod.h"
#include "context.h"
#include "hvs_struct.h"
#include "io_proxy/rpc_types.h"

extern bool zonechecker_run;
#define FUSE_DEBUG_LEVEL 30
#define HVS_FUSE_DATA \
  ((struct ::hvs::ClientFuseData *)fuse_get_context()->private_data)

using namespace std;
namespace fs = std::experimental::filesystem;
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
    // return -ENETUNREACH;
    return -EPERM;  // 当找不到远程路径时，说明当前的空间为不存在，返回无权限；
  }

  // if we have cache mod, try fetch it from cache mod first
  if (HVS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    auto found = HVS_FUSE_DATA->client->cache->get_stat(rpath, stbuf);
    if (found == ClientCache::FOUND) {
      dout(FUSE_DEBUG_LEVEL) << "cache mod hit stat :" << path << dendl;
      return 0;
    } else if (found == ClientCache::FOUND_MISSING) {
      dout(FUSE_DEBUG_LEVEL) << "cache mod hit stat missing :" << path << dendl;
      return -ENOENT;
    } else if (found == ClientCache::NOT_FOUND) {
      // wait remote fetch
    }
  }

  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::getattr);
  request.set_filepath(rpath);
  auto oper = HVS_FUSE_DATA->client->rpc->get_operator(iop);
  auto status = oper->Submit(request, reply);
  //  auto res = HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_stat", rpath);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }

  //  auto retbuf = res->as<ioproxy_rpc_statbuffer>();
  if (reply.err_code()) {
    // stat failed on remote server
    // if we have cache mod, try set it
    if (HVS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
      HVS_FUSE_DATA->client->cache->set_stat(rpath, nullptr);
      dout(FUSE_DEBUG_LEVEL) << "cache mod set missing stat :" << path << dendl;
    }
    return reply.err_code();
  }

  memset(stbuf, 0, sizeof(struct stat));
  //  stbuf->st_dev = static_cast<__dev_t>(retbuf.st_dev);
  //  stbuf->st_ino = static_cast<__ino_t>(retbuf.st_ino);
  stbuf->st_mode = static_cast<__mode_t>(reply.attr().permission());
  stbuf->st_nlink = 1;
  stbuf->st_uid = 1000;
  stbuf->st_gid = 1000;
  //  stbuf->st_rdev = static_cast<__dev_t>(retbuf.st_rdev);
  stbuf->st_size = reply.attr().size();
  //  stbuf->st_atim.tv_nsec = retbuf.st_atim_tv_nsec;
  // it's to diffcult to maintain the access time so I use change time to
  // replace it
  stbuf->st_atim.tv_sec = reply.attr().ctime();
  //  stbuf->st_mtim.tv_nsec = retbuf.st_mtim_tv_nsec;
  stbuf->st_mtim.tv_sec = reply.attr().mtime();
  //  stbuf->st_ctim.tv_nsec = retbuf.st_ctim_tv_nsec;
  stbuf->st_ctim.tv_sec = reply.attr().ctime();
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  // if we have cache mod, try set it
  if (HVS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    HVS_FUSE_DATA->client->cache->set_stat(rpath, stbuf);
    dout(FUSE_DEBUG_LEVEL) << "cache mod set stat :" << path << dendl;
  }
  return 0;
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

  //  auto res = HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_readdir",
  //  rpath);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::readdir);
  request.set_filepath(rpath);
  auto oper = HVS_FUSE_DATA->client->rpc->get_operator(iop);
  auto status = oper->Submit(request, reply);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }

  int retstat = 0;
  //  auto ents = res->as<std::vector<ioproxy_rpc_statbuffer>>();
  for (int i = 0; i < reply.entry_names_size(); i++) {
    //  for (const ioproxy_rpc_statbuffer &ent : ents) {
    const auto &attr = reply.entries(i);
    struct stat dirfilestat;
    memset(&dirfilestat, 0, sizeof(struct stat));
    //    dirfilestat.st_dev = static_cast<__dev_t>(ent.st_dev);
    //    dirfilestat.st_ino = static_cast<__ino_t>(ent.st_ino);
    dirfilestat.st_mode = static_cast<__mode_t>(attr.permission());
    dirfilestat.st_nlink = 1;
    dirfilestat.st_uid = 1000;
    dirfilestat.st_gid = 1000;
    //    dirfilestat.st_rdev = static_cast<__dev_t>(ent.st_rdev);
    dirfilestat.st_size = attr.size();
    //    dirfilestat.st_atim.tv_nsec = ent.st_atim_tv_nsec;
    dirfilestat.st_atim.tv_sec = attr.ctime();
    //    dirfilestat.st_mtim.tv_nsec = ent.st_mtim_tv_nsec;
    dirfilestat.st_mtim.tv_sec = attr.mtime();
    //    dirfilestat.st_ctim.tv_nsec = ent.st_ctim_tv_nsec;
    dirfilestat.st_ctim.tv_sec = attr.ctime();
    if (filler(buf, reply.entry_names(i).c_str(), &dirfilestat, 0,
               static_cast<fuse_fill_dir_flags>(0)) != 0) {
      return -ENOMEM;
    }
    if (HVS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
      HVS_FUSE_DATA->client->cache->set_stat(
          fs::path(rpath) / reply.entry_names(i), &dirfilestat);
      dout(FUSE_DEBUG_LEVEL) << "cache mod set stat :" << path << dendl;
    }
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
  if (HVS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    HVS_FUSE_DATA->client->cache->expire_stat(rpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  //  auto res = HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_open",
  //                                              rpath.c_str(), fi->flags);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::open);
  request.set_filepath(rpath);
  request.set_mode(fi->flags);
  auto oper = HVS_FUSE_DATA->client->rpc->get_operator(iop);
  auto status = oper->Submit(request, reply);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }
  retstat = reply.err_code();
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
  if (HVS_FUSE_DATA->fuse_client->readahead) {
    uint64_t cur_off = offset, size_left = size, read_size = 0,
             read_size_total = 0;
    while (size_left > 0) {
      uint64_t cur_sec_left =
          std::min(size_left, (((cur_off >> 18) + 1) << 18) - cur_off);
      auto cache_st = HVS_FUSE_DATA->client->readahead->status(
          rpath, cur_off, cur_sec_left, buf, read_size);
      if (cache_st == ClientReadAhead::IN_BUFFER) {
        dout(REAHAHEAD_DEBUG_LEVEL)
            << "cache hit: " << rpath << " sector: " << (offset >> 18) << dendl;
        // TODO: this is not real size
        cur_off += cur_sec_left;
        size_left -= cur_sec_left;
        read_size_total += read_size;
        continue;
      } else if (cache_st == ClientReadAhead::MAY_SEQ_READ) {
        bool nt = HVS_FUSE_DATA->client->readahead->set_task(
            iop, rpath, 0, offset >> 18, HVS_FUSE_DATA->fuse_client->readahead,
            -1);
        if (!nt) break;
      } else if (cache_st == ClientReadAhead::ON_LINK) {
        break;
      } else {
        break;
      }
    }
    if (size_left == 0) {
      return read_size_total;
    }
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
    //    auto res = HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_read",
    //    rpath,
    //                                                size, offset, fi->fh);
    OpRequest request;
    OpReply reply;
    request.set_type(OpType::read);
    request.set_filepath(rpath);
    request.mutable_io_param()->set_size(size);
    request.mutable_io_param()->set_offset(offset);
    auto oper = HVS_FUSE_DATA->client->rpc->get_operator(iop);
    auto status = oper->Submit(request, reply);
    if (!status.ok()) {
      // timeout exception raised
      return -ENOENT;
    }
    //    auto retbuf = res->as<ioproxy_rpc_buffer>();
    if (reply.err_code() <= 0) {
      // stat failed on remote server
      return reply.err_code();
    }
    memcpy(buf, reply.data().c_str(), reply.err_code());
    return reply.err_code();
  }
}

int hvsfs_write(const char *path, const char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi) {
  auto [iop, rpath] = HVS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }
  if (HVS_FUSE_DATA->fuse_client->readahead != 0) {
    HVS_FUSE_DATA->client->readahead->clear_buf(rpath);
  }
  if (HVS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    HVS_FUSE_DATA->client->cache->expire_stat(rpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
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
    //    auto res = HVS_FUSE_DATA->client->rpc->call(
    //        iop, "ioproxy_write", rpath.c_str(), _buffer, size, offset);
    OpRequest request;
    OpReply reply;
    request.set_type(OpType::write);
    request.set_filepath(rpath);
    request.mutable_io_param()->set_size(size);
    request.mutable_io_param()->set_offset(offset);
    request.set_data(buf, size);
    auto oper = HVS_FUSE_DATA->client->rpc->get_operator(iop);
    auto status = oper->Submit(request, reply);
    if (!status.ok()) {
      // timeout exception raised
      return -ENOENT;
    }
    //    auto retbuf = res->as<int>();
    return reply.err_code();
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
  //  auto res = HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_access",
  //                                              rpath.c_str(), mode);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::access);
  request.set_filepath(rpath);
  request.set_mode(mode);
  auto oper = HVS_FUSE_DATA->client->rpc->get_operator(iop);
  auto status = oper->Submit(request, reply);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return reply.err_code();
}

int hvsfs_opendir(const char *path, struct fuse_file_info *fi) {
  int retstat = 0;

  return retstat;
}

void hvsfs_destroy(void *private_data) { HVS_FUSE_DATA->client->stop(); }

int hvsfs_truncate(const char *path, off_t size, struct fuse_file_info *fi) {
  auto [iop, rpath] = HVS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }
  if (HVS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    HVS_FUSE_DATA->client->cache->expire_stat(rpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }
  //  auto res = HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_truncate",
  //                                              rpath.c_str(), offset);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::truncate);
  request.set_filepath(rpath);
  // truncate a file to specified length
  request.mutable_io_param()->set_size(size);
  auto oper = HVS_FUSE_DATA->client->rpc->get_operator(iop);
  auto status = oper->Submit(request, reply);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }
  //  auto retstat = res->as<int>();
  return reply.err_code();
}

int hvsfs_readlink(const char *path, char *link, size_t size) {
  //  auto [iop, rpath] = HVS_FUSE_DATA->client->graph->get_mapping(path);
  //  // not exists
  //  if (!iop) {
  //    return -ENOENT;
  //  }
  //
  //  auto res =
  //      HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_readlink", rpath,
  //      size);
  //  if (!res.get()) {
  //    // timeout exception raised
  //    return -ENOENT;
  //  }
  //
  //  auto retbuf = res->as<std::string>();
  //  memcpy(link, retbuf.c_str(), retbuf.size());
  //  return retbuf.size();
  return -ENOTSUP;
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

  if (HVS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    HVS_FUSE_DATA->client->cache->expire_stat(rpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  //  auto res = HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_mkdir",
  //                                              rpath.c_str(), mode);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::mkdir);
  request.set_filepath(rpath);
  request.set_mode(mode);
  auto oper = HVS_FUSE_DATA->client->rpc->get_operator(iop);
  auto status = oper->Submit(request, reply);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return reply.err_code();
}

int hvsfs_unlink(const char *path) {
  auto [iop, rpath] = HVS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }
  if (HVS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    HVS_FUSE_DATA->client->cache->expire_stat(rpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  //  auto res = HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_unlink", rpath);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::unlink);
  request.set_filepath(rpath);
  auto oper = HVS_FUSE_DATA->client->rpc->get_operator(iop);
  auto status = oper->Submit(request, reply);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return reply.err_code();
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
  if (HVS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    HVS_FUSE_DATA->client->cache->expire_stat(rpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  //  auto res = HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_rmdir", rpath);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::rmdir);
  request.set_filepath(rpath);
  auto oper = HVS_FUSE_DATA->client->rpc->get_operator(iop);
  auto status = oper->Submit(request, reply);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return reply.err_code();
}

int hvsfs_symlink(const char *path, const char *newpath) {
  int retstat = 0;
  dout(-1) << "TODO: 软连接操作有问题，待修复！" << dendl;
  //        std::string root("/");
  //        std::string paths(path);
  //        std::string fullpath = root+paths;
  //        auto ip = new std::string(ioproxy_ip.c_str());
  //        auto port =
  //        hvs::HvsContext::get_context()->_config->get<int>("rpc.port");
  //        RpcClient client(*ip, static_cast<const unsigned int>(*port));
  //        auto res = client.call("ioproxy_symlink", fullpath.c_str(),
  //        newpath); retstat = res->as<int>();
  return -ENOTSUP;
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
  if (HVS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    HVS_FUSE_DATA->client->cache->expire_stat(srpath);
    HVS_FUSE_DATA->client->cache->expire_stat(drpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  //  auto res =
  //      HVS_FUSE_DATA->client->rpc->call(siop, "ioproxy_rename", srpath,
  //      drpath);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::rename);
  request.set_filepath(srpath);
  request.set_newpath(drpath);
  auto oper = HVS_FUSE_DATA->client->rpc->get_operator(siop);
  auto status = oper->Submit(request, reply);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return reply.err_code();
}

int hvsfs_link(const char *path, const char *newpath) {
  int retstat = 0;
  dout(-1) << "TODO: 硬连接操作有问题，待修复！" << dendl;
  //        auto ip = new std::string(ioproxy_ip.c_str());
  //        auto port =
  //        hvs::HvsContext::get_context()->_config->get<int>("rpc.port");
  //        RpcClient client(*ip, static_cast<const unsigned int>(*port));
  //        auto res = client.call("ioproxy_link", path, newpath);
  //        retstat = res->as<int>();
  return -ENOTSUP;
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
  if (HVS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    HVS_FUSE_DATA->client->cache->expire_stat(rpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  //  auto res =
  //      HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_chmod", rpath, mode);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::chmod);
  request.set_filepath(rpath);
  request.set_mode(mode);
  auto oper = HVS_FUSE_DATA->client->rpc->get_operator(iop);
  auto status = oper->Submit(request, reply);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return reply.err_code();
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
  if (HVS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    HVS_FUSE_DATA->client->cache->expire_stat(rpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  //  auto res =
  //      HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_chown", rpath, uid,
  //      gid);
  //  if (!res.get()) {
  //    // timeout exception raised
  //    return -ENOENT;
  //  }
  //  auto retstat = res->as<int>();
  //  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return -ENOTSUP;
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
  if (HVS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    HVS_FUSE_DATA->client->cache->expire_stat(rpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }
  dout(FUSE_DEBUG_LEVEL) << "req-" << path << dendl;
  //  auto res =
  //      HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_create", rpath, mode);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::create);
  request.set_filepath(rpath);
  request.set_mode(mode);
  auto oper = HVS_FUSE_DATA->client->rpc->get_operator(iop);
  auto status = oper->Submit(request, reply);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote finish req: " << path << dendl;
  return reply.err_code();
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
  if (HVS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    HVS_FUSE_DATA->client->cache->expire_stat(rpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }
  //  auto res = HVS_FUSE_DATA->client->rpc->call(iop, "ioproxy_utimes", rpath,
  //                                              sec0n, sec0s, sec1n, sec1s);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::utimens);
  request.set_filepath(rpath);
  request.mutable_io_param()->set_size(sec1s);
  request.mutable_io_param()->set_offset(sec1n);
  auto oper = HVS_FUSE_DATA->client->rpc->get_operator(iop);
  auto status = oper->Submit(request, reply);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }
  return reply.err_code();
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
