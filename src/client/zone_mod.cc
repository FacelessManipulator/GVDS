#include <utility>
#include "client/clientuser/ClientUser.h"

//
// Created by yaowen on 5/28/19.
// 北航系统结构所-存储组
//

#include "client/graph_mod.h"
#include "client/msg_mod.h"
#include "zone_mod.h"

using namespace std;
using namespace gvds;

void gvds::ClientZone::start() {
  m_stop = false;
  create("client-zone-mod");
}

void gvds::ClientZone::stop() { m_stop = true; }

void *ClientZone::entry() {
  while (!m_stop) {
    auto client_id = client->user->getAccountID();
    if(client_id != "")
        client->zone->GetZoneInfo(client_id);
    std::this_thread::sleep_for(std::chrono::seconds(10));
  }
}

bool gvds::ClientZone::GetZoneInfo(std::string clientID) {
  vector<shared_ptr<Zone>> zoneinfores;
  string endpoint = client->get_manager();
  string inforesult =
      client->rpc->post_request(endpoint, "/zone/info", clientID);
  std::string routepath = "/users/isownerzone/" + clientID; ///users/search/用戶id
  std::string res = client->rpc->get_request(endpoint, routepath);
  if (!inforesult.empty()) {
    json_decode(inforesult, zoneinfores);  //获取返回的结果
  }
  if (zoneinfores.empty()) {
    return false;
  }
  spacemap_mutex.lock_shared();
  spaceuuid_to_metadatamap.clear();  // 进行清空map
  spacemap_mutex.unlock_shared();
  zonemap_mutex.lock();
  zonemap.clear();
  if(res=="true")
  {
    for (auto it : zoneinfores)
    {
      //TODO：admin获取所有空间信息，因为存在空间名重复，所以加入用户名标识唯一性
      zonemap[it->ownerID +"_"+ it->zoneName] = it;
      spacemap_mutex.lock_shared();
      for (auto sp : it->spaceBicInfo)
      {
        spaceuuid_to_metadatamap[sp->spaceID] = sp;
      }
      spacemap_mutex.unlock_shared();
    }
  }
  else
  {
    for (auto it : zoneinfores)
    {
      // TODO: 获取空间信息对每个空间，并更新到内存中；
      //        GetLocateInfo(clientID, it.zoneID, it.spaceBicInfo);
      zonemap[it->zoneName] = it;
      spacemap_mutex.lock_shared();
      for (auto sp : it->spaceBicInfo)
      {
        spaceuuid_to_metadatamap[sp->spaceID] = sp;
      }
      spacemap_mutex.unlock_shared();
    }
  }
  zonemap_mutex.unlock();
  return true;
}

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

std::tuple<std::shared_ptr<Zone>, std::shared_ptr<Space>, std::string> gvds::ClientZone::locatePosition(
    const std::string& path) {
  std::vector<std::string> namev = splitWithStl(path, "/");
  auto pos = path.find('/', 1);
  pos = path.find('/', pos + 1);
  std::string zonename = namev[1];
  std::string spacename = namev[2];
  std::string remotepath;
  std::string spaceuuid;
  if (pos == -1) {
    remotepath = "/";
  } else {
    remotepath = path.substr(pos);
  }
  lock_guard<mutex> lock(zonemap_mutex);
  auto mapping = zonemap.find(zonename);
  if (mapping != zonemap.end()) {
    auto zone = mapping->second;
    shared_ptr<Space> space;
    for (auto it : zone->spaceBicInfo) {
      if (it->spaceName == namev[2]) {
        space = it;
      }
    }
    return {zone, space, remotepath};
  } else {
    return {nullptr, nullptr, remotepath};
  }
}

std::tuple<std::string, std::shared_ptr<Zone>, std::shared_ptr<Space>, std::string> gvds::ClientZone::parsePath(
    const std::string& path) {
  std::vector<std::string> namev = splitWithStl(path, "/");
  auto pos = path.find('/', 1);
  pos = path.find('/', pos + 1);
  pos = path.find('/', pos + 1);
  std::string username = namev[1];
  std::string zonename = namev[2];
  std::string spacename = namev[3];
  std::string remotepath;
  std::string spaceuuid;
  if (pos == -1) {
    remotepath = "/";
  } else {
    remotepath = path.substr(pos);
  }
  lock_guard<mutex> lock(zonemap_mutex);
  auto mapping = zonemap.find(zonename);
  if (mapping != zonemap.end()) {
    auto zone = mapping->second;
    shared_ptr<Space> space;
    for (auto it : zone->spaceBicInfo) {
      if (it->spaceName == namev[3]) {
        space = it;
      }
    }
    return {username, zone, space, remotepath};
  } else {
    return {"", nullptr, nullptr, remotepath};
  }
}

std::string ClientZone::spaceuuid_to_spacerpath(std::string uuid) {
  std::string rpath;
  rpath = spaceuuid_to_metadatamap[uuid]->spacePath;
  return rpath;  // 返回存储集群路径
}

std::string ClientZone::spaceuuid_to_hostcenterID(std::string uuid) {
  std::string centerID;
  centerID = spaceuuid_to_metadatamap[uuid]->hostCenterID;
  return centerID;
}

int ClientZone::isZoneSpacePath(const string &path) {
  // TODO: 目前是通过路径判断当前是空间还是区域状态
  std::vector<std::string> namev = splitWithStl(path, "/");
  int nvsize = static_cast<int>(namev.size());
  // access space level
  if (strcmp(path.c_str(), "/") == 0) {
    return 0;
  }
  if (nvsize >= 2) {
    auto mapping = zonemap.find(namev[1]);
    if (mapping == zonemap.end()) {
      return -ENOENT;
    } else {
      if (nvsize == 2)
        return 1;  // is zone
      else if (nvsize == 3)
        return 2;  // is space
      else
        return 3;  // is remote node
    }
  }
}

int ClientZone::getattr(const char *path, struct stat *stbuf) {
  // init stat first
  auto t = time(nullptr);
  memset(stbuf, 0, sizeof(struct stat));
  stbuf->st_dev = 0;      // ignored
  stbuf->st_ino = 0;      // ignored
  stbuf->st_blksize = 0;  // ignored
  stbuf->st_mode = S_IFDIR | 0755;
  stbuf->st_nlink = 1;
  stbuf->st_size = 4096;
  stbuf->st_blocks = 8;
  stbuf->st_atim.tv_sec = t;
  stbuf->st_mtim.tv_sec = t;
  stbuf->st_ctim.tv_sec = t;

  std::vector<std::string> namev = splitWithStl(path, "/");
  int nvsize = static_cast<int>(namev.size());
  // root handle
  if (strcmp(path, "/") == 0) {
    stbuf->st_uid = 0;
    stbuf->st_gid = 0;
    return 0;
  }

  if (nvsize == 2 && !namev[1].empty()) {
    auto mapping = zonemap.find(namev[1]);
    if (mapping != zonemap.end()) {
      stbuf->st_mode = S_IFDIR | 0755;
      stbuf->st_uid = 1000 /*1000*/;  // TODO: 权限模块修改
      stbuf->st_gid = 1000 /*1000*/;  // TODO: 权限模块修改
      return 0;
    }
  }

  if (nvsize < 3) {
    return -ENOENT;
  }
}

int ClientZone::readdir(const char *path, void *buf, fuse_fill_dir_t filler) {
  std::vector<std::string> namev = splitWithStl(path, "/");
  int nvsize = static_cast<int>(namev.size());
  // access space level
  if (strcmp(path, "/") == 0) {
    std::vector<std::string> dirNames;
    // string endpoint = client->get_manager();
    // std::string routepath = "/users/isownerzone/" + client->user->getAccountID(); ///users/search/用戶id
    // std::string res = client->rpc->get_request(endpoint, routepath);
    // if(res=="true")
    //   dirNames = client->graph->list_owner_zone();
    // else
      dirNames = client->graph->list_zone();
    
    for (const auto &dirName : dirNames) {
      if (filler(buf, dirName.c_str(), nullptr, 0,
                 static_cast<fuse_fill_dir_flags>(0)) != 0) {
        return -ENOMEM;
      }
    }
    return 0;
  }

  if (nvsize == 2 && !namev[1].empty()) {
    auto spaces = client->graph->list_space(namev[1]);
    for (const auto &space : spaces) {
      if (filler(buf, space.c_str(), nullptr, 0,
                 static_cast<fuse_fill_dir_flags>(0)) != 0) {
        return -ENOMEM;
      }
    }
    return 0;
  }

  if (nvsize < 3) {
    return -ENOENT;
  }
  return 0;
}