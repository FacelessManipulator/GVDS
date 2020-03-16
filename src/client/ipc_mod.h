//
// Created by yaowen on 6/11/19.
// 北航系统结构所-存储组
//

#pragma once  // GVDS_IPC_MOD_H
#include <gvds_struct.h>
#include <pistache/client.h>
#include <future>
#include <mutex>
#include "client.h"
#include "client/clientuser/ClientAuth_struct.h"
#include "client/clientuser/ClientUser_struct.h"
#include "common/ipc/IPCServer.hpp"
#include "ipc_struct.h"

namespace gvds {
class ClientIPC : public ClientModule {
 private:
  virtual void start() override;
  virtual void stop() override;

 public:
  ClientIPC(const char *name, Client *cli) : ClientModule(name, cli) {
    isThread = true;
    init();
  }

  void init();

 private:
  friend class Client;
  std::shared_ptr<IPCServer> sp_ipcserver;
  std::unordered_map<std::string, Zone> zonemap;  // 区域重命名使用
  std::mutex mutex;

 private:
  // TODO: 客户端具体处理函数
  std::string dospacerename(IPCreq &ipcreq);  // 处理客户端函数
  std::string dospacerename_admin(IPCreq &ipcreq);
  std::string dospacesizechange(IPCreq &ipcreq);
  std::string dospaceusage(IPCreq &ipcreq);
  std::string dospaceusage_admin(IPCreq &ipcreq);
  std::string domapadd(IPCreq &ipcreq);
  std::string domapdeduct(IPCreq &ipcreq);
  std::string domapdeduct_admin(IPCreq &ipcreq);
  std::string dozoneadd_admin(IPCreq &ipcreq);
  std::string dozonecancel(IPCreq &ipcreq);
  std::string dozonecancel_admin(IPCreq &ipcreq);
  std::string dozoneregister(IPCreq &ipcreq);
  std::string dozonerename(IPCreq &ipcreq);
  std::string dozonerename_admin(IPCreq &ipcreq);
  std::string dozoneshare(IPCreq &ipcreq);
  std::string dozoneshare_admin(IPCreq &ipcreq);
  std::string dozonesharecancel(IPCreq &ipcreq);
  std::string dozonesharecancel_admin(IPCreq &ipcreq);
  std::string dozonelist(IPCreq &ipcreq);
  std::string dozonelist_admin(IPCreq &ipcreq);
  std::string dolistapply(IPCreq &ipcreq);
  std::string dosuggestion(IPCreq &ipcreq);
  std::string doadcam(IPCreq &ipcreq);
  std::string doaduam(IPCreq &ipcreq);
  std::string doadsearcham(IPCreq &ipcreq);
  std::string doadseepool(IPCreq &ipcreq);
  std::string doadauthsearch(IPCreq &ipcreq);
  std::string doadauthmodify(IPCreq &ipcreq);
  std::string dospacereplica(IPCreq &ipcreq);

  bool GetZoneInfo(std::string clientID);

  //资源聚合相关函数
  std::string doresourceregister(IPCreq &ipcreq);  //资源注册
  std::string doresourcedelete(IPCreq &ipcreq);    //资源删除
  std::string doresourcequery(IPCreq &ipcreq);     //资源查询
  std::string doresourceupdate(IPCreq &ipcreq);    //资源更新
  std::string doregisterupdate(IPCreq &ipcreq, std::string url);

  // user
  std::string douserlogin(IPCreq &ipcreq);
  std::string dousersearch(IPCreq &ipcreq);
  std::string dousersignup(IPCreq &ipcreq);
  std::string dousermodify(IPCreq &ipcreq);
  std::string douserexit(IPCreq &ipcreq);
  std::string dousercancel(IPCreq &ipcreq);

  std::string doauthsearch(IPCreq &ipcreq);
  std::string doauthmodify(IPCreq &ipcreq);

  std::string domodifycenter(IPCreq &ipcreq);
  std::string dosearchcenter(IPCreq &ipcreq);
  std::string dodeletecenter(IPCreq &ipcreq);

  std::string doadminsignup(IPCreq &ipcreq);
};
}  // namespace gvds

// GVDS_IPC_MOD_H
