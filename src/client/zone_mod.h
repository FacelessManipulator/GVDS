//
// Created by yaowen on 5/28/19.
// 北航系统结构所-存储组
//

#pragma once  // HVSONE_ZONE_MOD_H
#ifndef FUSE_USE_VERSION
#define FUSE_USE_VERSION 31
#endif
#include <fuse3/fuse.h>
#include <fuse3/fuse_lowlevel.h>
#include <pistache/client.h>
#include <unistd.h>
#include <shared_mutex>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <vector>
#include "client/client.h"
#include "hvs_struct.h"

// 主要包含区域信息检索前端模块和区域定位模块

namespace hvs {
class ClientZone : public ClientModule, public Thread {
 private:
  virtual void start() override;
  virtual void stop() override;
  void *entry() override;

 private:
  std::shared_mutex zonemap_mutex;
  std::shared_mutex spacemap_mutex;

 public:
  void check();
  ClientZone(const char *name, Client *cli)
      : ClientModule(name, cli), m_stop(true) {
    isThread = false;
  }
  //区域信息检索前端模块
  bool GetZoneInfo(
      std::string clientID);  // 区域信息检索前端模块，返回区域信息；
  std::string spaceuuid_to_spacerpath(
      std::string uuid);  // TODO: 直接通过uuid，获取到空间远程路径;
  std::string spaceuuid_to_hostcenterID(
      std::string uuid);  // TODO: 直接通过uuid，获取负责传输远程数据中心;
  std::unordered_map<std::string, std::shared_ptr<Zone>> zonemap;
  std::tuple<std::shared_ptr<Zone>, std::shared_ptr<Space>, std::string>
  locatePosition(const std::string& path);
  std::unordered_map<std::string, std::shared_ptr<Space>> spaceuuid_to_metadatamap;
  // 判断一个路径是否为区域、空间路径
  // <0, error; =0, not, >0 yes
  int isZoneSpacePath(const std::string &path);
  int getattr(const char *path, struct stat *stbuf);
  int readdir(const char *path, void *buf, fuse_fill_dir_t filler);

 private:
  friend class Client;
  bool m_stop;
};
}  // namespace hvs
// HVSONE_ZONE_MOD_H
