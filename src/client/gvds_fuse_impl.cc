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
#include "gvds_context.h"
#include "gvds_struct.h"
#include "io_proxy/rpc_types.h"
#include "common/centerinfo.h"
#include "OPTNode/opt_node.h"

extern bool zonechecker_run;
#define FUSE_DEBUG_LEVEL 30
#define GVDS_FUSE_DATA \
  ((struct ::gvds::ClientFuseData *)fuse_get_context()->private_data)

using namespace std;
namespace fs = std::experimental::filesystem;
namespace gvds {
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

double getBandWidth(const string & sourceCenterName,const string & destCenterName)
{
  std::map<std::string,int> name2Id={{"Beijing",0},{"Changsha",1},{"Shanghai",2},{"Jinan",3},{"Guangzhou",4},{"Beihang",5}};
  double bandWidth[6][6];
  bandWidth[0][0]=400000000;
  bandWidth[0][1]=4488693.75;
  bandWidth[0][2]=82083585.13;
  bandWidth[0][3]=112411237;
  bandWidth[0][4]=694813;
  bandWidth[0][5]=21775352.88;
  
  bandWidth[1][0]=30401278.5;
  bandWidth[1][1]=400000000;
  bandWidth[1][2]=16882664.13;
  bandWidth[1][3]=9725215.625;
  bandWidth[1][4]=4115036.125;
  bandWidth[1][5]=27114583.25;
  
  bandWidth[2][0]=53982675.75;
  bandWidth[2][1]=3843364.625;
  bandWidth[2][2]=400000000;
  bandWidth[2][3]=111804281.9;
  bandWidth[2][4]=1855816.375;
  bandWidth[2][5]=23148624;
  
  bandWidth[3][0]=112221802.6;
  bandWidth[3][1]=6191293.5;
  bandWidth[3][2]=25840320.13;
  bandWidth[3][3]=400000000;
  bandWidth[3][4]=3988432.875;
  bandWidth[3][5]=41293390.13;
  
  bandWidth[4][0]=3243403.25;
  bandWidth[4][1]=3609997.5;
  bandWidth[4][2]=1161078.625;
  bandWidth[4][3]=3481947.5;
  bandWidth[4][4]=400000000;
  bandWidth[4][5]=1292363.25;
  
  bandWidth[5][0]=28601205.88;
  bandWidth[5][1]=6284871;
  bandWidth[5][2]=6909725.5;
  bandWidth[5][3]=110698.25;
  bandWidth[5][4]=1618595;
  bandWidth[5][5]=400000000;
  return bandWidth[name2Id[sourceCenterName]][name2Id[destCenterName]];
}

void *gvdsfs_init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
  cfg->ac_attr_timeout_set = 1;
  cfg->attr_timeout = 1;
  cfg->entry_timeout = 1;
  cfg->negative_timeout = 1;
  conn->max_readahead = 524288;
  conn->max_read = 524288;
  return GVDS_FUSE_DATA;
}

// stat
int gvdsfs_getattr(const char *path, struct stat *stbuf,
                  struct fuse_file_info *fi) {
  auto zs_res = GVDS_FUSE_DATA->client->zone->isZoneSpacePath(path);
  // if path is not remote file
  if (zs_res < 0) {
    return zs_res;
  } else if (zs_res < 2) {
    return GVDS_FUSE_DATA->client->zone->getattr(path, stbuf);
  }

  auto [iop, rpath] = GVDS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    // return -ENETUNREACH;
    return -EPERM;  // 当找不到远程路径时，说明当前的空间为不存在，返回无权限；
  }

  // if we have cache mod, try fetch it from cache mod first
  if (GVDS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    auto found = GVDS_FUSE_DATA->client->cache->get_stat(rpath, stbuf);
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
  auto oper = GVDS_FUSE_DATA->client->rpc->get_operator(iop);
  auto status = oper->Submit(request, reply);
  //  auto res = GVDS_FUSE_DATA->client->rpc->call(iop, "ioproxy_stat", rpath);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }

  //  auto retbuf = res->as<ioproxy_rpc_statbuffer>();
  if (reply.err_code()) {
    // stat failed on remote server
    // if we have cache mod, try set it
    if (GVDS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
      GVDS_FUSE_DATA->client->cache->set_stat(rpath, nullptr);
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
  if (GVDS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    GVDS_FUSE_DATA->client->cache->set_stat(rpath, stbuf);
    dout(FUSE_DEBUG_LEVEL) << "cache mod set stat :" << path << dendl;
  }
  return 0;
}

int gvdsfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                  off_t offset, struct fuse_file_info *fi,
                  enum fuse_readdir_flags flags) {
  auto zs_res = GVDS_FUSE_DATA->client->zone->isZoneSpacePath(path);
  if (zs_res < 0) {
    return zs_res;
  } else if (zs_res < 2) {
    return GVDS_FUSE_DATA->client->zone->readdir(path, buf, filler);
  }

  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  auto [iop, rpath] = GVDS_FUSE_DATA->client->graph->get_mapping(path);

  // not exists
  if (!iop) {
    return -ENOENT;
  }

  //  auto res = GVDS_FUSE_DATA->client->rpc->call(iop, "ioproxy_readdir",
  //  rpath);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::readdir);
  request.set_filepath(rpath);
  auto oper = GVDS_FUSE_DATA->client->rpc->get_operator(iop);
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
    if (GVDS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
      GVDS_FUSE_DATA->client->cache->set_stat(
          fs::path(rpath) / reply.entry_names(i), &dirfilestat);
      dout(FUSE_DEBUG_LEVEL) << "cache mod set stat :" << path << dendl;
    }
  }
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;

  return retstat;
}

int gvdsfs_open(const char *path, struct fuse_file_info *fi) {
  int retstat = 0;
  int zs_res = GVDS_FUSE_DATA->client->zone->isZoneSpacePath(path);
  if (zs_res < 0) {
    return zs_res;
  } else if (zs_res < 2) {
    // zs_res < 0 or zs_res >0 both means zone mod handle this operation
    return -ENOTSUP;
  }
  auto [iop, rpath] = GVDS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }
  if (GVDS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    GVDS_FUSE_DATA->client->cache->expire_stat(rpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  //  auto res = GVDS_FUSE_DATA->client->rpc->call(iop, "ioproxy_open",
  //                                              rpath.c_str(), fi->flags);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::open);
  request.set_filepath(rpath);
  request.set_mode(fi->flags);
  auto oper = GVDS_FUSE_DATA->client->rpc->get_operator(iop);
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

int gvdsfs_read(const char *path, char *buf, size_t size, off_t offset,
               struct fuse_file_info *fi) {
  auto [iop, rpath] = GVDS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }
  if (GVDS_FUSE_DATA->fuse_client->readahead) {
    uint64_t cur_off = offset, size_left = size, read_size = 0,
             read_size_total = 0;
    while (size_left > 0) {
      uint64_t cur_sec_left =
          std::min(size_left, (((cur_off >> 18) + 1) << 18) - cur_off);
      auto cache_st = GVDS_FUSE_DATA->client->readahead->status(
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
        bool nt = GVDS_FUSE_DATA->client->readahead->set_task(
            iop, rpath, 0, offset >> 18, GVDS_FUSE_DATA->fuse_client->readahead,
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

  if (GVDS_FUSE_DATA->fuse_client->use_udt) {
    // UDT version
    auto res = GVDS_FUSE_DATA->client->rpc->read_data(iop, _buffer);
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
    //    auto res = GVDS_FUSE_DATA->client->rpc->call(iop, "ioproxy_read",
    //    rpath,
    //                                                size, offset, fi->fh);
    OpRequest request;
    OpReply reply;
    request.set_type(OpType::read);
    request.set_filepath(rpath);
    request.mutable_io_param()->set_size(size);
    request.mutable_io_param()->set_offset(offset);
    auto oper = GVDS_FUSE_DATA->client->rpc->get_operator(iop);
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

int gvdsfs_write(const char *path, const char *buf, size_t size, off_t offset,
                struct fuse_file_info *fi) {
  auto [iop, rpath] = GVDS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }
  if (GVDS_FUSE_DATA->fuse_client->readahead != 0) {
    GVDS_FUSE_DATA->client->readahead->clear_buf(rpath);
  }
  if (GVDS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    GVDS_FUSE_DATA->client->cache->expire_stat(rpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }

  ioproxy_rpc_buffer _buffer(rpath.c_str(), buf, offset, size);
  _buffer.is_read = false;
  _buffer.fid = fi->fh;
  _buffer.flags = fi->flags;

  if (GVDS_FUSE_DATA->fuse_client->async_mode) {
    auto buf2q = std::make_shared<gvds::Buffer>(rpath, buf, offset, size);
    buf2q->dest = iop;
    GVDS_FUSE_DATA->client->queue->queue_buffer(buf2q);
    return size;
  } else {
    // tcp version
    //    auto res = GVDS_FUSE_DATA->client->rpc->call(
    //        iop, "ioproxy_write", rpath.c_str(), _buffer, size, offset);
    OpRequest request;
    OpReply reply;
    request.set_type(OpType::write);
    request.set_filepath(rpath);
    request.mutable_io_param()->set_size(size);
    request.mutable_io_param()->set_offset(offset);
    request.set_data(buf, size);
    auto oper = GVDS_FUSE_DATA->client->rpc->get_operator(iop);
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

int gvdsfs_access(const char *path, int mode) {
  int zs_res = GVDS_FUSE_DATA->client->zone->isZoneSpacePath(path);
  if (zs_res < 0) {
    return zs_res;
  } else if (zs_res < 2) {
    return 0;
  }
  auto [iop, rpath] = GVDS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  //  auto res = GVDS_FUSE_DATA->client->rpc->call(iop, "ioproxy_access",
  //                                              rpath.c_str(), mode);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::access);
  request.set_filepath(rpath);
  request.set_mode(mode);
  auto oper = GVDS_FUSE_DATA->client->rpc->get_operator(iop);
  auto status = oper->Submit(request, reply);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return reply.err_code();
}

int gvdsfs_opendir(const char *path, struct fuse_file_info *fi) {
  int retstat = 0;

  return retstat;
}

void gvdsfs_destroy(void *private_data) { GVDS_FUSE_DATA->client->stop(); }

int gvdsfs_truncate(const char *path, off_t size, struct fuse_file_info *fi) {
  auto [iop, rpath] = GVDS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }
  if (GVDS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    GVDS_FUSE_DATA->client->cache->expire_stat(rpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }
  //  auto res = GVDS_FUSE_DATA->client->rpc->call(iop, "ioproxy_truncate",
  //                                              rpath.c_str(), offset);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::truncate);
  request.set_filepath(rpath);
  // truncate a file to specified length
  request.mutable_io_param()->set_size(size);
  auto oper = GVDS_FUSE_DATA->client->rpc->get_operator(iop);
  auto status = oper->Submit(request, reply);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }
  //  auto retstat = res->as<int>();
  return reply.err_code();
}

int gvdsfs_readlink(const char *path, char *link, size_t size) {
  //  auto [iop, rpath] = GVDS_FUSE_DATA->client->graph->get_mapping(path);
  //  // not exists
  //  if (!iop) {
  //    return -ENOENT;
  //  }
  //
  //  auto res =
  //      GVDS_FUSE_DATA->client->rpc->call(iop, "ioproxy_readlink", rpath,
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

int gvdsfs_mknod(const char *path, mode_t mode, dev_t dev) {
  int retstat = 0;

  return retstat;
}
int gvdsfs_mkdir(const char *path, mode_t mode) {
  std::vector<std::string> namev = splitWithStl(path, "/");
  int nvsize = static_cast<int>(namev.size());
  if (namev.size() <= 3) {
    return -EPERM;
  }
  auto [iop, rpath] = GVDS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }

  if (GVDS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    GVDS_FUSE_DATA->client->cache->expire_stat(rpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  //  auto res = GVDS_FUSE_DATA->client->rpc->call(iop, "ioproxy_mkdir",
  //                                              rpath.c_str(), mode);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::mkdir);
  request.set_filepath(rpath);
  request.set_mode(mode);
  auto oper = GVDS_FUSE_DATA->client->rpc->get_operator(iop);
  auto status = oper->Submit(request, reply);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return reply.err_code();
}

int gvdsfs_unlink(const char *path) {
  auto [iop, rpath] = GVDS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }
  if (GVDS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    GVDS_FUSE_DATA->client->cache->expire_stat(rpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  //  auto res = GVDS_FUSE_DATA->client->rpc->call(iop, "ioproxy_unlink", rpath);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::unlink);
  request.set_filepath(rpath);
  auto oper = GVDS_FUSE_DATA->client->rpc->get_operator(iop);
  auto status = oper->Submit(request, reply);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return reply.err_code();
}

int gvdsfs_rmdir(const char *path) {
  //跳过本地
  std::vector<std::string> namev = splitWithStl(path, "/");
  int nvsize = static_cast<int>(namev.size());
  if (namev.size() <= 3) {
    return -EPERM;
  }
  // 删除远程
  auto [iop, rpath] = GVDS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }
  if (GVDS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    GVDS_FUSE_DATA->client->cache->expire_stat(rpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  //  auto res = GVDS_FUSE_DATA->client->rpc->call(iop, "ioproxy_rmdir", rpath);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::rmdir);
  request.set_filepath(rpath);
  auto oper = GVDS_FUSE_DATA->client->rpc->get_operator(iop);
  auto status = oper->Submit(request, reply);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return reply.err_code();
}

int gvdsfs_symlink(const char *path, const char *newpath) {
  int retstat = 0;
  dout(-1) << "TODO: 软连接操作有问题，待修复！" << dendl;
  //        std::string root("/");
  //        std::string paths(path);
  //        std::string fullpath = root+paths;
  //        auto ip = new std::string(ioproxy_ip.c_str());
  //        auto port =
  //        gvds::HvsContext::get_context()->_config->get<int>("rpc.port");
  //        RpcClient client(*ip, static_cast<const unsigned int>(*port));
  //        auto res = client.call("ioproxy_symlink", fullpath.c_str(),
  //        newpath); retstat = res->as<int>();
  return -ENOTSUP;
}

int gvdsfs_rename(const char *path, const char *newpath, unsigned int flags) {
  //跳过本地
  std::vector<std::string> namev = splitWithStl(path, "/");
  int nvsize = static_cast<int>(namev.size());
  if (namev.size() <= 3) {
    return -EPERM;
  }

  // access content level
  auto [siop, srpath] = GVDS_FUSE_DATA->client->graph->get_mapping(path);
  auto [diop, drpath] = GVDS_FUSE_DATA->client->graph->get_mapping(newpath);
  // not exists
  if (!siop || !diop) {
    return -ENOENT;
  } else if (siop->uuid != diop->uuid) {
    // over ioproxy move is not support yet
    return -ENOTSUP;
  }
  if (GVDS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    GVDS_FUSE_DATA->client->cache->expire_stat(srpath);
    GVDS_FUSE_DATA->client->cache->expire_stat(drpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  //  auto res =
  //      GVDS_FUSE_DATA->client->rpc->call(siop, "ioproxy_rename", srpath,
  //      drpath);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::rename);
  request.set_filepath(srpath);
  request.set_newpath(drpath);
  auto oper = GVDS_FUSE_DATA->client->rpc->get_operator(siop);
  auto status = oper->Submit(request, reply);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return reply.err_code();
}

int gvdsfs_link(const char *path, const char *newpath) {
  int retstat = 0;
  dout(-1) << "TODO: 硬连接操作有问题，待修复！" << dendl;
  //        auto ip = new std::string(ioproxy_ip.c_str());
  //        auto port =
  //        gvds::HvsContext::get_context()->_config->get<int>("rpc.port");
  //        RpcClient client(*ip, static_cast<const unsigned int>(*port));
  //        auto res = client.call("ioproxy_link", path, newpath);
  //        retstat = res->as<int>();
  return -ENOTSUP;
}

int gvdsfs_chmod(const char *path, mode_t mode, struct fuse_file_info *fi) {
  std::vector<std::string> namev = splitWithStl(path, "/");
  int nvsize = static_cast<int>(namev.size());
  if (namev.size() <= 3) {
    return -EPERM;
  }
  auto [iop, rpath] = GVDS_FUSE_DATA->client->graph->get_mapping(path);

  // not exists
  if (!iop) {
    return -ENOENT;
  }
  if (GVDS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    GVDS_FUSE_DATA->client->cache->expire_stat(rpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  //  auto res =
  //      GVDS_FUSE_DATA->client->rpc->call(iop, "ioproxy_chmod", rpath, mode);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::chmod);
  request.set_filepath(rpath);
  request.set_mode(mode);
  auto oper = GVDS_FUSE_DATA->client->rpc->get_operator(iop);
  auto status = oper->Submit(request, reply);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return reply.err_code();
}

int gvdsfs_chown(const char *path, uid_t uid, gid_t gid,
                struct fuse_file_info *fi) {
  std::vector<std::string> namev = splitWithStl(path, "/");
  int nvsize = static_cast<int>(namev.size());
  if (namev.size() <= 3) {
    return -EPERM;
  }
  auto [iop, rpath] = GVDS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }
  if (GVDS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    GVDS_FUSE_DATA->client->cache->expire_stat(rpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote req-" << path << dendl;
  //  auto res =
  //      GVDS_FUSE_DATA->client->rpc->call(iop, "ioproxy_chown", rpath, uid,
  //      gid);
  //  if (!res.get()) {
  //    // timeout exception raised
  //    return -ENOENT;
  //  }
  //  auto retstat = res->as<int>();
  //  dout(FUSE_DEBUG_LEVEL) << "remote finish req-" << path << dendl;
  return -ENOTSUP;
}

int gvdsfs_statfs(const char *, struct statvfs *) {
  int retstat = 0;

  return retstat;
}

int gvdsfs_flush(const char *, struct fuse_file_info *) {
  int retstat = 0;

  return retstat;
}

int gvdsfs_release(const char *, struct fuse_file_info *) {
  int retstat = 0;
  return retstat;
}

int gvdsfs_fsync(const char *, int, struct fuse_file_info *) {
  int retstat = 0;

  return retstat;
}

int gvdsfs_releasedir(const char *, struct fuse_file_info *) {
  int retstat = 0;

  return retstat;
}

int gvdsfs_create(const char *path, mode_t mode, struct fuse_file_info *) {
  std::vector<std::string> namev = splitWithStl(path, "/");
  int nvsize = static_cast<int>(namev.size());
  if (namev.size() <= 3) {
    return -EPERM;
  }
  auto [iop, rpath] = GVDS_FUSE_DATA->client->graph->get_mapping(path);
  // not exists
  if (!iop) {
    return -ENOENT;
  }
  if (GVDS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    GVDS_FUSE_DATA->client->cache->expire_stat(rpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }
  dout(FUSE_DEBUG_LEVEL) << "req-" << path << dendl;
  //  auto res =
  //      GVDS_FUSE_DATA->client->rpc->call(iop, "ioproxy_create", rpath, mode);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::create);
  request.set_filepath(rpath);
  request.set_mode(mode);
  auto oper = GVDS_FUSE_DATA->client->rpc->get_operator(iop);
  auto status = oper->Submit(request, reply);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }
  dout(FUSE_DEBUG_LEVEL) << "remote finish req: " << path << dendl;
  return reply.err_code();
}

int gvdsfs_utimens(const char *path, const struct timespec tv[2],
                  struct fuse_file_info *fi) {
  std::vector<std::string> namev = splitWithStl(path, "/");
  int nvsize = static_cast<int>(namev.size());
  if (namev.size() <= 3) {
    return -EPERM;
  }
  auto [iop, rpath] = GVDS_FUSE_DATA->client->graph->get_mapping(path);
  // access content level
  long int sec0n = tv[0].tv_nsec;
  long int sec0s = tv[0].tv_sec;
  long int sec1n = tv[1].tv_nsec;
  long int sec1s = tv[1].tv_sec;
  // not exists
  if (!iop) {
    return -ENOENT;
  }
  if (GVDS_FUSE_DATA->client->cache->max_stat_cache_ct > 0) {
    GVDS_FUSE_DATA->client->cache->expire_stat(rpath);
    dout(FUSE_DEBUG_LEVEL) << "cache mod expire stat :" << path << dendl;
  }
  //  auto res = GVDS_FUSE_DATA->client->rpc->call(iop, "ioproxy_utimes", rpath,
  //                                              sec0n, sec0s, sec1n, sec1s);
  OpRequest request;
  OpReply reply;
  request.set_type(OpType::utimens);
  request.set_filepath(rpath);
  request.mutable_io_param()->set_size(sec1s);
  request.mutable_io_param()->set_offset(sec1n);
  auto oper = GVDS_FUSE_DATA->client->rpc->get_operator(iop);
  auto status = oper->Submit(request, reply);
  if (!status.ok()) {
    // timeout exception raised
    return -ENOENT;
  }
  return reply.err_code();
}

    int gvdsfs_setxattr(const char *path, const char* name, const char* value, size_t size, int flags) {
        std::vector<std::string> namev = splitWithStl(path, "/");
        int nvsize = static_cast<int>(namev.size());
        if (namev.size() <= 3) {
            return -EPERM;
        }

        if (string(name) == "user.gvds.visit.force") {
          GVDS_FUSE_DATA->client->graph->set_force_mapping(path, string(value, size));
          dout(-1) << "set force visit space at center " << string(value, size) << dendl;
          return 0;
        }

        auto [iop, rpath] = GVDS_FUSE_DATA->client->graph->get_mapping(path);
        // not exists
        if (!iop) {
            return -ENOENT;
        }
        dout(FUSE_DEBUG_LEVEL) << "req-" << path << dendl;
        //  auto res =
        //      GVDS_FUSE_DATA->client->rpc->call(iop, "ioproxy_create", rpath, mode);
        OpRequest request;
        OpReply reply;
        request.set_type(OpType::setxattr);
        request.set_filepath(rpath);
        request.set_xattr_name(name);
        request.set_data(value, size);
        request.set_size(size);
        request.set_mode(flags);
        auto oper = GVDS_FUSE_DATA->client->rpc->get_operator(iop);
        auto status = oper->Submit(request, reply);
        if (!status.ok()) {
            // timeout exception raised
            return -ENOENT;
        }
        dout(FUSE_DEBUG_LEVEL) << "remote finish req: " << path << dendl;
        return reply.err_code();
    }

    int gvdsfs_getxattr(const char *path, const char* name, char* value, size_t size) {
        std::vector<std::string> namev = splitWithStl(path, "/");
        if (namev.size() <= 3) {
            return -ENODATA;
        }
        auto [iop, rpath] = GVDS_FUSE_DATA->client->graph->get_mapping(path);
        // not exists
        if (!iop) {
            return -ENODATA;
        }
        dout(FUSE_DEBUG_LEVEL) << "req-" << path << " name:"<< name << dendl;

        std::vector<std::string> attrlist = splitWithStl(name, ".");
        if (attrlist.size()==3 && attrlist[0]=="gvds" && attrlist[1]=="attr" && attrlist[2]=="size")
        {
          OpRequest request;
          OpReply reply;
          request.set_type(OpType::getattr);
          request.set_filepath(rpath);
          auto oper = GVDS_FUSE_DATA->client->rpc->get_operator(iop);
          auto status = oper->Submit(request, reply);
          if (!status.ok()) {
            // timeout exception raised
            return -ENOENT;
          }
          if (reply.err_code()) {
            return reply.err_code();
          }
          std::string size_str=std::to_string(reply.attr().size());
          if(value==NULL)
          {
            value=new char [size_str.size()];
          }
          memcpy(value, size_str.c_str(), size_str.size());
          dout(FUSE_DEBUG_LEVEL) << "remote finish req: " << path << dendl;
          return size_str.size();
        }
        else if(attrlist.size()==4 && attrlist[0]=="gvds" && attrlist[1]=="migrate")
        {
          std::string destCenterName = attrlist[2];
          std::string destCenterId;
          std::string cinfor = GVDS_FUSE_DATA->client->optNode->getCenterInfo();
          CenterInfo mycenter;
          mycenter.deserialize(cinfor);
          bool isin=false;
          for(auto centertmp:mycenter.centerName)
          {
            if(centertmp.second == destCenterName)
            {
              isin=true;
              destCenterId=centertmp.first;
              break;
            }
          }
          if(!isin)
            return -ENODATA;
          auto [zone, space, remotepath] = GVDS_FUSE_DATA->client->zone->locatePosition(path);
          std::string sourceCenterName = space->hostCenterName;


          if(attrlist[3]=="bandwidth")
          {
            std::string bandWidth=std::to_string(getBandWidth(sourceCenterName,destCenterName));
            if(value==NULL)
            {
              value=new char [bandWidth.size()];
            }
            memcpy(value, bandWidth.c_str(), bandWidth.size());
            return bandWidth.size();
          }
          else if(attrlist[3]=="time")
          {
            OpRequest request;
            OpReply reply;
            request.set_type(OpType::getattr);
            request.set_filepath(rpath);
            auto oper = GVDS_FUSE_DATA->client->rpc->get_operator(iop);
            auto status = oper->Submit(request, reply);
            if (!status.ok()) {
              // timeout exception raised
              return -ENOENT;
            }
            if (reply.err_code()) {
              return reply.err_code();
            }
            auto file_size=reply.attr().size();
            double bandWidth=getBandWidth(sourceCenterName,destCenterName);
            std::string mig_time=std::to_string(file_size/bandWidth);
            if(value==NULL)
            {
              value=new char [mig_time.size()];
            }
            memcpy(value, mig_time.c_str(), mig_time.size());
            dout(FUSE_DEBUG_LEVEL) << "remote finish req: " << path << dendl;
            return mig_time.size();
          }
          else if(attrlist[3]=="isok")
          {
            std::string isok="true";
            if(value==NULL)
            {
              value=new char [isok.size()];
              memcpy(value, isok.c_str(), isok.size());
            }
            else
            {  
              memcpy(value, isok.c_str(), isok.size());
            }
            return isok.size();
          }
        }

        if (string(name).rfind("security.capability") == 0) {
          return -ENODATA;
        }
        //  auto res =
        //      GVDS_FUSE_DATA->client->rpc->call(iop, "ioproxy_create", rpath, mode);
        OpRequest request;
        OpReply reply;
        request.set_type(OpType::getxattr);
        request.set_filepath(rpath);
        request.set_xattr_name(name);
        request.set_size(size);
        auto oper = GVDS_FUSE_DATA->client->rpc->get_operator(iop);
        auto status = oper->Submit(request, reply);
        if (!status.ok()) {
            // timeout exception raised
            return -ENOENT;
        }
        if (reply.err_code() > 0) {
            memcpy(value, reply.data().c_str(), reply.err_code());
        }
        dout(FUSE_DEBUG_LEVEL) << "remote finish req: " << path << dendl;
        return reply.err_code();
    }

    int gvdsfs_listxattr(const char *path, char* value, size_t size) {
        std::vector<std::string> namev = splitWithStl(path, "/");
        int nvsize = static_cast<int>(namev.size());
        if (namev.size() <= 3) {
            return -EPERM;
        }
        auto [iop, rpath] = GVDS_FUSE_DATA->client->graph->get_mapping(path);
        // not exists
        if (!iop) {
            return -ENOENT;
        }
        dout(FUSE_DEBUG_LEVEL) << "req-" << path << dendl;
        //  auto res =
        //      GVDS_FUSE_DATA->client->rpc->call(iop, "ioproxy_create", rpath, mode);
        OpRequest request;
        OpReply reply;
        request.set_type(OpType::listxattr);
        request.set_filepath(rpath);
        request.set_size(size);
        auto oper = GVDS_FUSE_DATA->client->rpc->get_operator(iop);
        auto status = oper->Submit(request, reply);
        if (!status.ok()) {
            // timeout exception raised
            return -ENOENT;
        }
        if (reply.err_code() > 0) {
            memcpy(value, reply.data().c_str(), reply.err_code());
        }
        dout(FUSE_DEBUG_LEVEL) << "remote finish req: " << path << dendl;
        return reply.err_code();
    }

    int gvdsfs_removexattr(const char *path, const char* name) {
        std::vector<std::string> namev = splitWithStl(path, "/");
        int nvsize = static_cast<int>(namev.size());
        if (namev.size() <= 3) {
            return -EPERM;
        }
        auto [iop, rpath] = GVDS_FUSE_DATA->client->graph->get_mapping(path);
        // not exists
        if (!iop) {
            return -ENOENT;
        }
        dout(FUSE_DEBUG_LEVEL) << "req-" << path << dendl;
        //  auto res =
        //      GVDS_FUSE_DATA->client->rpc->call(iop, "ioproxy_create", rpath, mode);
        OpRequest request;
        OpReply reply;
        request.set_type(OpType::removexattr);
        request.set_filepath(rpath);
        request.set_xattr_name(name);
        auto oper = GVDS_FUSE_DATA->client->rpc->get_operator(iop);
        auto status = oper->Submit(request, reply);
        if (!status.ok()) {
            // timeout exception raised
            return -ENOENT;
        }
        dout(FUSE_DEBUG_LEVEL) << "remote finish req: " << path << dendl;
        return reply.err_code();
    }

struct fuse_operations gvdsfs_oper = {
    .getattr = gvdsfs_getattr,
    .readlink = gvdsfs_readlink,
    .mknod = gvdsfs_mknod,  // TODO: create函数重复，暂时不做
    .mkdir = gvdsfs_mkdir,
    .unlink = gvdsfs_unlink,
    .rmdir = gvdsfs_rmdir,
    .symlink = gvdsfs_symlink,  // TODO: 软连接操作暂时有问题，待修复；
    .rename = gvdsfs_rename,
    .link = gvdsfs_link,  // TODO: 硬连接操作暂时有问题，待修复；
    .chmod =
        gvdsfs_chmod,  // TODO: 虚拟数据空间相关，涉及到权限，之后需要统一修改；
    .chown =
        gvdsfs_chown,  // TODO: 虚拟数据空间相关，涉及到权限，之后需要统一修改；
    .truncate = gvdsfs_truncate,
    .open = gvdsfs_open,
    .read = gvdsfs_read,
    .write = gvdsfs_write,
    .statfs = gvdsfs_statfs,    // TODO: vfs 相关，暂时不做
    .flush = gvdsfs_flush,      // TODO: cache 相关，暂时不做
    .release = gvdsfs_release,  // TODO: 未保存文件fd, 暂时不做
    .fsync = gvdsfs_fsync,      // TODO: caced 相关暂时不做
    .setxattr = gvdsfs_setxattr,
    .getxattr = gvdsfs_getxattr,
    .listxattr = gvdsfs_listxattr,
    .removexattr = gvdsfs_removexattr,
    .opendir = gvdsfs_opendir,
    .readdir = gvdsfs_readdir,
    .releasedir = gvdsfs_releasedir,  // TODO: 未保存DIR, 暂时不做
    .fsyncdir = NULL,
    .init = gvdsfs_init,
    .destroy = gvdsfs_destroy,
    .access = gvdsfs_access,
    .create = gvdsfs_create,
    .lock = NULL,
    .utimens = gvdsfs_utimens,
    .bmap = NULL,
    .ioctl = NULL,
    .poll = NULL,
    .write_buf = NULL,
    .read_buf = NULL,
    .flock = NULL,
    .fallocate = NULL,
};
}  // namespace gvds
