#include <stdio.h>
#include <iostream>
#include <vector>
#include "common/JsonSerializer.h"
#include "context.h"
#include "datastore/datastore.h"

#include <pistache/client.h>
#include <pistache/http.h>
#include <pistache/net.h>
#include <atomic>
#include <future>

#include "common/centerinfo.h"
#include "common/json.h"
#include "manager/usermodel/Account.h"
#include "manager/usermodel/UserModelServer.h"
#include "manager/zone/ZoneServer.h"

bool isSubset(std::vector<std::string> v1, std::vector<std::string> v2) {
  int i = 0, j = 0;
  int m = v1.size();
  int n = v2.size();
  if (m < n) {
    return 0;
  }
  sort(v1.begin(), v1.end());
  sort(v2.begin(), v2.end());
  while (i < n && j < m) {
    if (v1[j] < v2[i]) {
      j++;
    } else if (v1[j] == v2[i]) {
      j++;
      i++;
    } else if (v1[j] > v2[i]) {
      return 0;
    }
  }
  if (i < n) {
    return 0;
  } else {
    return 1;
  }
}

namespace gvds {
using namespace Pistache::Rest;
using namespace Pistache::Http;

void ZoneServer::start() {}

void ZoneServer::stop() {}

void ZoneServer::router(Router &router) {
  Routes::Post(router, "/zone/rename",
               Routes::bind(&ZoneServer::ZoneRenameRest, this));
  Routes::Post(router, "/zone/locate",
               Routes::bind(&ZoneServer::GetZoneLocateInfoRest, this));
  Routes::Post(router, "/zone/info",
               Routes::bind(&ZoneServer::GetZoneInfoRest, this));
  Routes::Post(router, "/zone/share",
               Routes::bind(&ZoneServer::ZoneShareRest, this));
  Routes::Post(router, "/zone/sharecancel",
               Routes::bind(&ZoneServer::ZoneShareCancelRest, this));
  Routes::Post(router, "/zone/registerapply",
               Routes::bind(&ZoneServer::ZoneRegisterApplyRest, this));
  Routes::Post(router, "/zone/registercheck",
               Routes::bind(&ZoneServer::ZoneRegisterCheckRest, this));
  Routes::Post(router, "/zone/register",
               Routes::bind(&ZoneServer::ZoneRegisterRest, this));
  Routes::Post(router, "/zone/cancel",
               Routes::bind(&ZoneServer::ZoneCancelRest, this));
  Routes::Post(router, "/zone/mapaddapply",
               Routes::bind(&ZoneServer::MapAddApplyRest, this));
  Routes::Post(router, "/zone/mapaddcheck",
               Routes::bind(&ZoneServer::MapAddCheckRest, this));
  Routes::Post(router, "/zone/mapadd",
               Routes::bind(&ZoneServer::MapAddRest, this));
  Routes::Post(router, "/zone/mapdeduct",
               Routes::bind(&ZoneServer::MapDeductRest, this));
  Routes::Post(router, "/zone/add",
               Routes::bind(&ZoneServer::ZoneAddRest, this));
}

//区域重命名
void ZoneServer::ZoneRenameRest(const Rest::Request &request,
                                Http::ResponseWriter response) {
  dout(10) << "====== start ZoneServer function: ZoneRenameRest ======"
           << dendl;
  auto info = request.body();

  ZoneRequest req;
  req.deserialize(info);
  std::string zoneID = req.zoneID;
  std::string ownerID = req.ownerID;
  std::string newZoneName = req.newZoneName;

  int result = ZoneRename(zoneID, ownerID, newZoneName);
  response.send(Http::Code::Ok, json_encode(result));
  dout(10) << "====== end ZoneServer function: ZoneRenameRest ======" << dendl;
}

int ZoneServer::ZoneRename(std::string zoneID, std::string ownerID,
                           std::string newZoneName) {
  Zone tmp;
  std::shared_ptr<gvds::Datastore> zonePtr =
      gvds::DatastoreFactory::create_datastore(
          zonebucket, gvds::DatastoreType::couchbase, true);
  auto [vp, err] = zonePtr->get(zone_prefix + zoneID);
  if (err) {
    return EAGAIN;
  }
  std::string tmp_value = *vp;
  tmp.deserialize(tmp_value);
  UserModelServer *p_usermodel =
      static_cast<UserModelServer *>(mgr->get_module("user").get());
  if (tmp.ownerID == ownerID ||
      p_usermodel->validadminidentity(ownerID))  // TODO:添加管理员ID
  {
    if (newZoneName.size() > 50) {
      newZoneName = newZoneName.substr(50);
    }
    char query[256];
    snprintf(query, 256,
             "select UUID, name, owner, members, spaces from `%s` where owner "
             "= \"%s\" and name = \"%s\" and META().id like '%s%%'",
             zonebucket.c_str(), ownerID.c_str(), newZoneName.c_str(),
             zone_prefix.c_str());
    // std::shared_ptr<gvds::Datastore> dbPtr =
    // gvds::DatastoreFactory::create_datastore(zonebucket,
    // gvds::DatastoreType::couchbase, true);
    auto zonePtr2 = static_cast<CouchbaseDatastore *>(zonePtr.get());
    auto [vp, err] = zonePtr2->n1ql(string(query));
    if (vp->size() != 0) {
      return EINVAL;
    }
    tmp.zoneName = std::move(newZoneName);
    tmp.contains_spaceinfo = false;
    if (zonePtr->set(zone_prefix + zoneID, tmp.serialize()) != 0)
      return EAGAIN;  //插入报错
    else
      return 0;
  } else
    return EACCES;
}

//区域定位
void ZoneServer::GetZoneLocateInfoRest(const Rest::Request &request,
                                       Http::ResponseWriter response) {
  dout(10) << "====== start ZoneServer function: GetZoneLocateInfoRest ======"
           << dendl;
  auto info = request.body();

  ZoneRequest req;
  req.deserialize(info);
  std::string clientID = req.clientID;
  std::string zoneID = req.zoneID;
  std::vector<std::string> spaceID = req.spaceID;
  std::vector<Space> result_zl;

  bool result_b = GetZoneLocateInfo(result_zl, clientID, zoneID, spaceID);
  std::string result;
  if (result_b) {
    result = json_encode(result_zl);
  } else {
    result = "fail";
  }
  response.send(Http::Code::Ok, result);
  dout(10) << "====== end ZoneServer function: GetZoneLocateInfoRest ======"
           << dendl;
}

bool ZoneServer::GetZoneLocateInfo(std::vector<Space> &result,
                                   std::string clientID, std::string zoneID,
                                   std::vector<std::string> spaceID) {
  Zone tmp;
  std::shared_ptr<gvds::Datastore> zonePtr =
      gvds::DatastoreFactory::create_datastore(
          zonebucket, gvds::DatastoreType::couchbase, true);
  zonePtr->init();
  auto [vz, err] = zonePtr->get(zone_prefix + zoneID);
  if (err) {
    return false;
  }
  std::string tmp_value = *vz;
  tmp.deserialize(tmp_value);
  UserModelServer *p_usermodel =
      static_cast<UserModelServer *>(mgr->get_module("user").get());
  if (clientID == tmp.ownerID ||
      std::find(tmp.memberID.begin(), tmp.memberID.end(), clientID) !=
          tmp.memberID.end() ||
      p_usermodel->validadminidentity(clientID))  // clientID是zone的成员或主人
  {
    if (isSubset(tmp.spaceID, spaceID)) {
      SpaceServer *tmp_server =
          static_cast<SpaceServer *>(mgr->get_module("space").get());
      tmp_server->GetSpacePosition(result, spaceID);
      return true;
    } else
      return false;
  } else
    return false;
}

//区域信息检索
void ZoneServer::GetZoneInfoRest(const Rest::Request &request,
                                 Http::ResponseWriter response) {
  dout(15) << "INFO: ZoneServer: GetZoneInfoRest request." << dendl;
  auto info = request.body();
  std::string clientID = info;
  std::vector<Zone> result_z;

  bool result_b = GetZoneInfo(result_z, clientID);
  std::string result;
  if (!result_z.empty()) {
    result = json_encode(result_z);
  } else {
    result = "fail";
  }
  response.send(Http::Code::Ok, result);
}

bool ZoneServer::GetZoneInfo(std::vector<Zone> &result_z,
                             std::string clientID) {
  //查找是owner的区域
  char query[256];
  snprintf(query, 256,
           "select UUID, name, owner, members, spaces from `%s` where owner = "
           "\"%s\" and META().id like '%s%%'",
           zonebucket.c_str(), clientID.c_str(), zone_prefix.c_str());

  std::shared_ptr<gvds::Datastore> accountPtr =
      gvds::DatastoreFactory::create_datastore(
          accountbucket, gvds::DatastoreType::couchbase, true);
  std::shared_ptr<gvds::Datastore> dbPtr =
      gvds::DatastoreFactory::create_datastore(
          zonebucket, gvds::DatastoreType::couchbase, true);
  auto zonePtr = static_cast<CouchbaseDatastore *>(dbPtr.get());
  auto [vp, err] = zonePtr->n1ql(string(query));
  if (err) {
    return false;
  }
  //查找是member的区域
  char query2[256];
  snprintf(query2, 256,
           "select UUID, name, owner, members, spaces from `%s` where \"%s\" "
           "within members and META().id like '%s%%'",
           zonebucket.c_str(), clientID.c_str(), zone_prefix.c_str());

  std::shared_ptr<gvds::Datastore> dbPtr2 =
      gvds::DatastoreFactory::create_datastore(
          zonebucket, gvds::DatastoreType::couchbase, true);
  auto zonePtr2 = static_cast<CouchbaseDatastore *>(dbPtr2.get());
  auto [vp2, err2] = zonePtr2->n1ql(string(query2));
  if (err2) {
    return false;
  }
  vp->insert(vp->end(), vp2->begin(), vp2->end());

    for (std::vector<std::string>::iterator it = vp->begin(); it != vp->end();
         it++) {
      Zone tmp;
      Zone tmp_zi;

      std::string tmp_value = *it;
      tmp.deserialize(tmp_value);
      tmp_zi.zoneID = tmp.zoneID;
      tmp_zi.zoneName = tmp.zoneName;
      // id赋值name
      auto [own, oerr] = accountPtr->get(user_prefix + tmp.ownerID);
      if (!oerr) {
        Account owner;
        owner.deserialize(*own);
        tmp_zi.ownerID = owner.accountName;
      } else
        return false;

      // memberid赋值
      for (std::vector<std::string>::iterator m = tmp.memberID.begin();
           m != tmp.memberID.end(); m++) {
        auto [mem, merr] = accountPtr->get(user_prefix + *m);
        if (!merr) {
          Account member;
          member.deserialize(*mem);
          tmp_zi.memberID.push_back(member.accountName);
        } else
          return false;
      }
      std::vector<Space> result_s;
      SpaceServer *tmp_server =
          dynamic_cast<SpaceServer *>(mgr->get_module("space").get());
      tmp_server->GetSpaceInfo(result_s, tmp.spaceID);
      for (auto &result : result_s) {
        if (result.status == false) {
          continue;
        } else {
          tmp_zi.spaceBicInfo.push_back(make_shared<Space>(result));
        }
      }
      for (auto &sp : tmp_zi.spaceBicInfo) {
        tmp_zi.spaceID.push_back(sp->spaceID);
      }
      result_z.push_back(tmp_zi);
    }
    return true;
}

//区域共享
void ZoneServer::ZoneShareRest(const Rest::Request &request,
                               Http::ResponseWriter response) {
  dout(10) << "====== start ZoneServer function: ZoneShareRest ======" << dendl;
  auto info = request.body();
  ZoneRequest req;
  req.deserialize(info);

  int result = ZoneShare(req.zoneID, req.ownerID, req.memberID);
  response.send(Http::Code::Ok, json_encode(result));
  dout(10) << "====== end ZoneServer function: ZoneShareRest ======" << dendl;
}

int ZoneServer::ZoneShare(std::string zoneID, std::string ownerID,
                          std::vector<std::string> memberID) {
  Zone tmp;
  std::shared_ptr<gvds::Datastore> zonePtr =
      gvds::DatastoreFactory::create_datastore(
          zonebucket, gvds::DatastoreType::couchbase, true);
  zonePtr->init();
  auto [vp, err] = zonePtr->get(zone_prefix + zoneID);
  if (err) {
    return EAGAIN;
  }
  std::string tmp_value = *vp;
  tmp.deserialize(tmp_value);
  UserModelServer *p_usermodel =
      static_cast<UserModelServer *>(mgr->get_module("user").get());
  if (tmp.ownerID == ownerID ||
      p_usermodel->validadminidentity(ownerID))  // TODO:添加管理员ID
  {
    for (std::vector<std::string>::iterator it = memberID.begin();
         it != memberID.end(); it++) {
      std::string tmp_mem = *it;
      if (std::find(tmp.memberID.begin(), tmp.memberID.end(), tmp_mem) !=
          tmp.memberID.end())
        continue;
      else
        tmp.memberID.emplace_back(tmp_mem);
    }
    tmp_value = tmp.serialize();
    tmp.contains_spaceinfo = false;
    int flag = zonePtr->set(zone_prefix + zoneID, tmp_value);
    if (flag != 0)
      return EAGAIN;
    else
      return 0;
  } else
    return EACCES;
}

void ZoneServer::ZoneShareCancelRest(const Rest::Request &request,
                                     Http::ResponseWriter response) {
  dout(10) << "====== start ZoneServer function: ZoneShareCancelRest ======"
           << dendl;
  auto info = request.body();
  ZoneRequest req;
  req.deserialize(info);

  int result = ZoneShareCancel(req.zoneID, req.ownerID, req.memberID);
  response.send(Http::Code::Ok, json_encode(result));
  dout(10) << "====== end ZoneServer function: ZoneShareCancelRest ======"
           << dendl;
}

int ZoneServer::ZoneShareCancel(std::string zoneID, std::string ownerID,
                                std::vector<std::string> memberID) {
  Zone tmp;
  std::shared_ptr<gvds::Datastore> zonePtr =
      gvds::DatastoreFactory::create_datastore(
          zonebucket, gvds::DatastoreType::couchbase, true);
  zonePtr->init();
  auto [vp, err] = zonePtr->get(zone_prefix + zoneID);
  if (err != 0) {
    dout(10) << "未找到对应的区域" << dendl;
    return EAGAIN;
  }
  std::string tmp_value = *vp;
  tmp.deserialize(tmp_value);
  if (!isSubset(tmp.memberID, memberID)) {
    return EINVAL;
  }
  UserModelServer *p_usermodel =
      static_cast<UserModelServer *>(mgr->get_module("user").get());
  if (tmp.ownerID == ownerID ||
      p_usermodel->validadminidentity(ownerID))  // TODO:添加管理员ID
  {
    for (std::vector<std::string>::iterator it = memberID.begin();
         it != memberID.end(); it++) {
      std::string tmp_mem = *it;
      std::vector<std::string>::iterator m = tmp.memberID.begin();
      while (m != tmp.memberID.end()) {
        if (*m == tmp_mem) {
          m = tmp.memberID.erase(m);
        } else {
          ++m;
        }
      }
    }
    tmp_value = tmp.serialize();
    tmp.contains_spaceinfo = false;
    int flag = zonePtr->set(zone_prefix + zoneID, tmp_value);
    if (flag != 0)
      return EAGAIN;
    else
      return 0;
  } else
    return EACCES;
}

//区域注册--管理员通过审批后调用
void ZoneServer::ZoneRegisterCheckRest(const Rest::Request &request,
                                       Http::ResponseWriter response) {
  dout(10) << "====== start ZoneServer function: ZoneRegisterCheckRest ======"
           << dendl;
  auto info = request.body();
  ZoneRequest req;
  req.deserialize(info);
  Space tmpm;
  tmpm.deserialize(req.spacePathInfo);
  if (ManagerID == tmpm.hostCenterID || tmpm.hostCenterID == "") {
    int result = ZoneRegister(req.zoneName, req.ownerID, req.memberID,
                              req.spaceName, req.spaceSize, req.spacePathInfo);
    response.send(Http::Code::Ok, json_encode(result));
  } else {
    std::shared_ptr<gvds::CouchbaseDatastore> f0_dbPtr =
        std::make_shared<gvds::CouchbaseDatastore>(
            gvds::CouchbaseDatastore(accountbucket));
    f0_dbPtr->init();
    string c_key = "center_information";
    auto [pcenter_value, c_error] = f0_dbPtr->get(c_key);
    if (c_error) {
      dout(10) << "authmodelserver: get center_information fail" << dendl;
      errno = EAGAIN;
      response.send(Http::Code::Ok, json_encode(errno));
      return;
    }
    CenterInfo mycenter;
    mycenter.deserialize(*pcenter_value);

    Http::Client client;
    char url[256];

    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);

    string tmp_ip = mycenter.centerIP[tmpm.hostCenterID];
    string tmp_port = mycenter.centerPort[tmpm.hostCenterID];

    snprintf(url, 256, "http://%s:%s/zone/register", tmp_ip.c_str(),
             tmp_port.c_str());

    auto response2 = client.post(url).body(info).send();
    std::promise<bool> prom;
    auto fu = prom.get_future();
    response2.then(
        [&](Http::Response resp) {
          // dout(-1) << "Manager Info: " << res.body() << dendl;
          dout(10) << "Response code = " << resp.code() << dendl;
          auto body = resp.body();
          response.send(Http::Code::Ok, body);
          prom.set_value(true);
        },
        Async::IgnoreException);

    //阻塞
    fu.get();
    client.shutdown();
  }

  dout(10) << "====== end ZoneServer function: ZoneRegisterCheckRest ======"
           << dendl;
}
//跨域调用的接口
void ZoneServer::ZoneRegisterRest(const Rest::Request &request,
                                  Http::ResponseWriter response) {
  dout(10) << "====== start ZoneServer function: ZoneRegisterRest ======"
           << dendl;
  auto info = request.body();
  ZoneRequest req;
  req.deserialize(info);
  // std::string globalManageNodeInfo =req.globalManageNodeInfo;客户端判断

  dout(10) << "info: " << info << dendl;

  int result = ZoneRegister(req.zoneName, req.ownerID, req.memberID,
                            req.spaceName, req.spaceSize, req.spacePathInfo);
  response.send(Http::Code::Ok, json_encode(result));
  dout(10) << "====== end ZoneServer function: ZoneRegisterRest ======"
           << dendl;
}

int ZoneServer::ZoneRegister(std::string zoneName, std::string ownerID,
                             std::vector<std::string> memberID,
                             std::string spaceName, int64_t spaceSize,
                             std::string spacePathInfo) {
  std::shared_ptr<gvds::Datastore> dbPtr =
      gvds::DatastoreFactory::create_datastore(
          zonebucket, gvds::DatastoreType::couchbase, true);
  auto zonePtr = static_cast<CouchbaseDatastore *>(dbPtr.get());
  //插入判断，加的这个区域是否已经存在
  char query[256];
  snprintf(query, 256,
           "select UUID, name, owner, members, spaces from `%s` where owner = "
           "\"%s\" and name = \"%s\" and META().id like '%s%%'",
           zonebucket.c_str(), ownerID.c_str(), zoneName.c_str(),
           zone_prefix.c_str());

  auto [vp, err] = zonePtr->n1ql(string(query));
  if (vp->size() != 0) {
    return EINVAL;
  } else {
    Zone tmp;

    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    const std::string tmp_uuid = boost::uuids::to_string(a_uuid);
    tmp.zoneID = tmp_uuid;
    std::string groupname = tmp_uuid.substr(0, 9);
    // 1、TODO:
    // 调用spacecreate接口（涉及到跨域创建空间的情况，则返还客户端，并再次发送）
    // , 目前在区域初始注册的时候，只能创建一个默认的空间
    //跨域空间创建情况，考虑采用各超算管各自的创建，本超算不成功则返回客户端发送请求到下一顺位
    // 2、TODO：调用权限模块
    SpaceServer *tmp_server = dynamic_cast<SpaceServer *>(
        mgr->get_module("space").get());  //获取空间服务端模块
    std::string res_sc =
        tmp_server->SpaceCreate(std::move(spaceName), ownerID, memberID,
                                spaceSize, std::move(spacePathInfo), groupname);
    if (res_sc == "-1") {
      return ENOSPC;
    }
    if (res_sc == "-2") {
      return EAGAIN;
    }
    if (res_sc == "-3") {
      return ENOENT;
    }
    if (res_sc == "-4") {
      return EACCES;
    }
    if (res_sc == "-5") {
      return EINVAL;
    } else {
      std::string spaceID = res_sc;
      tmp.zoneName = std::move(zoneName);
      tmp.ownerID = ownerID;
      tmp.memberID = memberID;
      tmp.spaceID.emplace_back(spaceID);
      tmp.contains_spaceinfo = false;
      if (tmp.zoneName == "") return EINVAL;
      int flag = dbPtr->set(zone_prefix + tmp.zoneID, tmp.serialize());
      if (flag != 0) return EAGAIN;
      return 0;
    }
  }
}

//管理员区域添加
void ZoneServer::ZoneAddRest(const Rest::Request &request,
                             Http::ResponseWriter response) {
  dout(10) << "====== start ZoneServer function: ZoneAddRest ======" << dendl;
  auto info = request.body();
  ZoneRequest req;
  req.deserialize(info);
  // std::string globalManageNodeInfo =req.globalManageNodeInfo;客户端判断

  int result =
      ZoneAdd(req.zoneName, req.ownerID, req.memberID, req.spacePathInfo);

  response.send(Http::Code::Ok, json_encode(result));  // point
  dout(10) << "====== end ZoneServer function: ZoneAddRest ======" << dendl;
}

int ZoneServer::ZoneAdd(std::string zoneName, std::string ownerID,
                        std::vector<std::string> memberID,
                        std::string spacePathInfo) {
  Zone tmp;
  std::shared_ptr<gvds::Datastore> dbPtr =
      gvds::DatastoreFactory::create_datastore(
          zonebucket, gvds::DatastoreType::couchbase, true);
  auto zonePtr = static_cast<CouchbaseDatastore *>(dbPtr.get());
  //插入判断，加的这个区域是否已经存在
  char query[256];
  snprintf(query, 256,
           "select UUID, name, owner, members, spaces from `%s` where owner = "
           "\"%s\" and name = \"%s\" and META().id like '%s%%'",
           zonebucket.c_str(), ownerID.c_str(), zoneName.c_str(),
           zone_prefix.c_str());

  auto [vp, err] = zonePtr->n1ql(string(query));
  if (err != 0) {
    dout(10) << "数据库连接失败" << dendl;
    return EAGAIN;
  }
  if (vp->size() == 0) {
    SpaceServer *tmp_server = dynamic_cast<SpaceServer *>(
        mgr->get_module("space").get());  //获取空间服务端
    std::string res_sc =
        tmp_server->SpaceCheck(ownerID, memberID, std::move(spacePathInfo));
    if (res_sc == "false") {
      return EINVAL;
    } else {
      std::string spaceID = res_sc;
      boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
      const std::string tmp_uuid = boost::uuids::to_string(a_uuid);
      tmp.zoneID = tmp_uuid;
      tmp.zoneName = std::move(zoneName);
      tmp.ownerID = ownerID;
      tmp.memberID = memberID;
      tmp.spaceID.emplace_back(spaceID);

      tmp.contains_spaceinfo = false;
      int flag = dbPtr->set(zone_prefix + tmp.zoneID, tmp.serialize());
      if (flag != 0) return EAGAIN;
      return 0;
    }
  } else if (vp->size() == 1) {
    SpaceServer *tmp_server = dynamic_cast<SpaceServer *>(
        mgr->get_module("space").get());  //获取空间服务端
    std::string res_sc =
        tmp_server->SpaceCheck(ownerID, memberID, std::move(spacePathInfo));
    if (res_sc == "false") {
      return EINVAL;
    } else {
      std::string spaceID = res_sc;
      std::vector<std::string>::iterator it = vp->begin();
      std::string tmp_value = *it;
      tmp.deserialize(tmp_value);
      tmp.spaceID.emplace_back(spaceID);
      tmp.contains_spaceinfo = false;
      int flag2 = dbPtr->set(zone_prefix + tmp.zoneID, tmp.serialize());
      if (flag2 != 0) return EAGAIN;
      return 0;
    }
  } else
    return EINVAL;
}

//区域注销
void ZoneServer::ZoneCancelRest(const Rest::Request &request,
                                Http::ResponseWriter response) {
  dout(10) << "====== start ZoneServer function: ZoneCancelRest ======"
           << dendl;
  auto info = request.body();
  dout(10) << info << dendl;
  ZoneRequest req;
  req.deserialize(info);

  int result = ZoneCancel(req.zoneID, req.ownerID);
  response.send(Http::Code::Ok, json_encode(result));
  dout(10) << "====== end ZoneServer function: ZoneCancelRest ======" << dendl;
}

int ZoneServer::ZoneCancel(std::string zoneID, std::string ownerID) {
  Zone tmp;
  std::shared_ptr<gvds::Datastore> zonePtr =
      gvds::DatastoreFactory::create_datastore(
          zonebucket, gvds::DatastoreType::couchbase, true);
  auto [vp, err] = zonePtr->get(zone_prefix + zoneID);
  if (err != 0) {
    return EAGAIN;
  }
  if (vp->size() == 0) {
    return ENOENT;
  }
  std::string tmp_value = *vp;
  tmp.deserialize(tmp_value);
  UserModelServer *p_usermodel =
      static_cast<UserModelServer *>(mgr->get_module("user").get());
  if (tmp.ownerID == ownerID ||
      p_usermodel->validadminidentity(ownerID))  // TODO:添加管理员ID
  {
    SpaceServer *tmp_server =
        dynamic_cast<SpaceServer *>(mgr->get_module("space").get());
    if (tmp_server->SpaceDelete(tmp.spaceID) == 0) {
      int flag = zonePtr->remove(zone_prefix + zoneID);
      if (flag != 0) return EAGAIN;
      return 0;
    } else
      return EAGAIN;
  } else
    return EACCES;
}

//区域映射编辑--管理员通过审批后调用
void ZoneServer::MapAddCheckRest(const Rest::Request &request,
                                 Http::ResponseWriter response) {
  dout(10) << "====== start ZoneServer function: MapAddCheckRest ======"
           << dendl;
  auto info = request.body();
  ZoneRequest req;
  req.deserialize(info);
  Space tmpm;
  tmpm.deserialize(req.spacePathInfo);
  if (ManagerID == tmpm.hostCenterID || tmpm.hostCenterID == "") {
    int result = MapAdd(req.zoneID, req.ownerID, req.spaceName, req.spaceSize,
                        req.spacePathInfo);
    response.send(Http::Code::Ok, json_encode(result));  // point
  } else {
    std::shared_ptr<gvds::CouchbaseDatastore> f0_dbPtr =
        std::make_shared<gvds::CouchbaseDatastore>(
            gvds::CouchbaseDatastore(accountbucket));
    f0_dbPtr->init();
    string c_key = "center_information";
    auto [pcenter_value, c_error] = f0_dbPtr->get(c_key);
    if (c_error) {
      dout(10) << "authmodelserver: get center_information fail" << dendl;
      errno = EAGAIN;
      response.send(Http::Code::Ok, json_encode(errno));
      return;
    }
    CenterInfo mycenter;
    mycenter.deserialize(*pcenter_value);

    Http::Client client;
    char url[256];

    auto opts = Http::Client::options().threads(1).maxConnectionsPerHost(8);
    client.init(opts);

    string tmp_ip = mycenter.centerIP[tmpm.hostCenterID];
    string tmp_port = mycenter.centerPort[tmpm.hostCenterID];

    snprintf(url, 256, "http://%s:%s/zone/mapadd", tmp_ip.c_str(),
             tmp_port.c_str());

    auto response2 = client.post(url).body(info).send();
    std::promise<bool> prom;
    auto fu = prom.get_future();
    response2.then(
        [&](Http::Response resp) {
          // dout(-1) << "Manager Info: " << res.body() << dendl;
          dout(10) << "Response code = " << resp.code() << dendl;
          auto body = resp.body();
          response.send(Http::Code::Ok, body);
          prom.set_value(true);
        },
        Async::IgnoreException);

    //阻塞
    fu.get();
    client.shutdown();
  }

  dout(10) << "====== end ZoneServer function: MapAddCheckRest ======" << dendl;
}
//跨域调用接口
void ZoneServer::MapAddRest(const Rest::Request &request,
                            Http::ResponseWriter response) {
  dout(10) << "====== start ZoneServer function: MapAddRest ======" << dendl;
  auto info = request.body();
  ZoneRequest req;
  req.deserialize(info);
  // std::string globalManageNodeInfo =req.globalManageNodeInfo;客户端判断
  dout(10) << "info: " << info << dendl;
  int result = MapAdd(req.zoneID, req.ownerID, req.spaceName, req.spaceSize,
                      req.spacePathInfo);

  response.send(Http::Code::Ok, json_encode(result));  // point
  dout(10) << "====== end ZoneServer function: MapAddRest ======" << dendl;
}

int ZoneServer::MapAdd(std::string zoneID, std::string ownerID,
                       std::string spaceName, int64_t spaceSize,
                       std::string spacePathInfo) {
  Zone tmp;
  std::shared_ptr<gvds::Datastore> zonePtr =
      gvds::DatastoreFactory::create_datastore(
          zonebucket, gvds::DatastoreType::couchbase, true);
  std::string tmp_key = zone_prefix + zoneID;
  auto [vp, err] = zonePtr->get(tmp_key);
  if (err) {
    return EAGAIN;
  }
  std::string tmp_value = *vp;
  tmp.deserialize(tmp_value);
  std::vector<Space> tmps;
  Space spaceurl;
  spaceurl.deserialize(spacePathInfo);
  SpaceServer *tmp_server =
      static_cast<SpaceServer *>(mgr->get_module("space").get());
  tmp_server->GetSpacePosition(tmps, tmp.spaceID);
  for (std::vector<Space>::iterator m = tmps.begin(); m != tmps.end(); m++) {
    if (m->spaceName == spaceName) {
      return EINVAL;
    }
  }

  if (tmp.ownerID == ownerID) {
    std::vector<std::string> memberID = tmp.memberID;
    //调用方法
    // SpaceServer* tmp_server = gvds::SpaceServer::getInstance();
    std::string groupname = zoneID.substr(0, 9);
    std::string res_sc = tmp_server->SpaceCreate(
        spaceName, ownerID, memberID, spaceSize, spacePathInfo, groupname);
    if (res_sc == "-1") {
      return ENOSPC;
    }
    if (res_sc == "-2") {
      return EAGAIN;
    }
    if (res_sc == "-3") {
      return ENOENT;
    }
    if (res_sc == "-4") {
      return EACCES;
    }
    if (res_sc == "-5") {
      return EINVAL;
    } else {
      //得到返回的空间ID，并加入到区域的包含的空间ID的向量中；
      std::string spaceID = res_sc;
      tmp.spaceID.emplace_back(spaceID);
      tmp.contains_spaceinfo = false;
      int flag = zonePtr->set(tmp_key, tmp.serialize());
      if (flag != 0) return EAGAIN;
      return 0;
    }
  }
}

void ZoneServer::MapDeductRest(const Rest::Request &request,
                               Http::ResponseWriter response) {
  dout(10) << "====== start ZoneServer function: MapDeductRest ======" << dendl;
  auto info = request.body();
  ZoneRequest req;
  req.deserialize(info);

  dout(10) << "info: " << info << dendl;

  int result = MapDeduct(req.zoneID, req.ownerID, req.spaceID);
  response.send(Http::Code::Ok, json_encode(result));
  dout(10) << "====== end ZoneServer function: MapDeductRest ======" << dendl;
}

int ZoneServer::MapDeduct(std::string zoneID, std::string ownerID,
                          std::vector<std::string> spaceID) {
  Zone tmp;
  std::shared_ptr<gvds::Datastore> zonePtr =
      gvds::DatastoreFactory::create_datastore(
          zonebucket, gvds::DatastoreType::couchbase, true);
  auto [vp, err] = zonePtr->get(zone_prefix + zoneID);
  if (err) {
    return EAGAIN;
  }
  std::string tmp_value = *vp;
  tmp.deserialize(tmp_value);
  if (tmp.ownerID == ownerID) {
    if (isSubset(tmp.spaceID, spaceID)) {
      SpaceServer *tmp_server =
          dynamic_cast<SpaceServer *>(mgr->get_module("space").get());
      tmp_server->SpaceDelete(spaceID);  // 调用空间模块，删除空间条目
      for (std::vector<std::string>::iterator it = spaceID.begin();
           it != spaceID.end(); it++) {
        std::string will_removed_spaceID = *it;
        std::vector<std::string>::iterator m = tmp.spaceID.begin();
        while (m != tmp.spaceID.end()) {
          if (*m == will_removed_spaceID) {
            m = tmp.spaceID.erase(m);
          } else {
            ++m;
          }
        }
      }
      tmp.contains_spaceinfo = false;
      int flag = zonePtr->set(zone_prefix + zoneID, tmp.serialize());
      if (flag != 0) return EAGAIN;
      return 0;
    } else
      return EINVAL;
  } else
    return EACCES;
}

void ZoneServer::ZoneRegisterApplyRest(const Rest::Request &request,
                                       Http::ResponseWriter response) {
  dout(10) << "====== start ZoneServer function: ZoneRegisterApplyRest ======"
           << dendl;
  auto info = request.body();

  dout(10) << "info: " << info << dendl;

  int result = ZoneRegisterApply(info);
  response.send(Http::Code::Ok, json_encode(result));
  dout(10) << "====== end ZoneServer function: ZoneRegisterApplyRest ======"
           << dendl;
}

int ZoneServer::ZoneRegisterApply(std::string apply) {
  std::shared_ptr<gvds::Datastore> applyPtr =
      gvds::DatastoreFactory::create_datastore(
          applybucket, gvds::DatastoreType::couchbase, true);
  boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
  const std::string uuid = boost::uuids::to_string(a_uuid);
  std::string key = "zregi-" + uuid;
  struct_apply_info applyinfo;
  applyinfo.id = key;
  ZoneRequest req;
  req.deserialize(apply);
  if (req.zoneName.size() > 50)  //限制长度为50
  {
    req.zoneName = req.zoneName.substr(50);
  }
  if (req.spaceName.size() > 50)  //限制长度为50
  {
    req.spaceName = req.spaceName.substr(50);
  }
  applyinfo.data = req.serialize();
  std::string value = applyinfo.serialize();
  int flag = applyPtr->set(key, value);
  if (flag != 0) {
    return EAGAIN;
  } else
    return 0;
}

void ZoneServer::MapAddApplyRest(const Rest::Request &request,
                                 Http::ResponseWriter response) {
  dout(10) << "====== start ZoneServer function: MapAddApplyRest ======"
           << dendl;
  auto info = request.body();

  dout(10) << "info: " << info << dendl;

  int result = MapAddApply(info);
  response.send(Http::Code::Ok, json_encode(result));
  dout(10) << "====== end ZoneServer function: MapAddApplyRest ======" << dendl;
}
int ZoneServer::MapAddApply(std::string apply) {
  std::shared_ptr<gvds::Datastore> applyPtr =
      gvds::DatastoreFactory::create_datastore(
          applybucket, gvds::DatastoreType::couchbase, true);
  boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
  const std::string uuid = boost::uuids::to_string(a_uuid);
  std::string key = "spadd-" + uuid;
  struct_apply_info applyinfo;
  applyinfo.id = key;
  ZoneRequest req;
  req.deserialize(apply);
  if (req.spaceName.size() > 50)  //限制长度为50
  {
    req.spaceName = req.spaceName.substr(50);
  }
  applyinfo.data = req.serialize();
  std::string value = applyinfo.serialize();
  int flag = applyPtr->set(key, value);
  if (flag != 0) {
    return EAGAIN;
  } else
    return 0;
}
}  // namespace gvds
