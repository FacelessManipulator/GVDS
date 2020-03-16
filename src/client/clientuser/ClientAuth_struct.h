#ifndef CLIENTAUTH_H
#define CLIENTAUTH_H

#include "common/JsonSerializer.h"
#include "context.h"
#include <map>  

class Auth: public gvds::JsonSerializer {
public:
    std::string zoneID;
    std::string GVDSAccountID;
    int owner_read;
    int owner_write;
    int owner_exe;

    int group_read;
    int group_write;
    int group_exe;

    int other_read;
    int other_write;
    int other_exe;

public:
  void serialize_impl() override;
  void deserialize_impl() override;

public:
    Auth() = default;
    Auth(std::string id) : zoneID(id){}
};


//请求self  rest 所需的结构
class SelfAuthSpaceInfo: public gvds::JsonSerializer {
public:
    std::string spaceinformation;
    std::string ownerID_zone;
    std::vector<std::string> memberID;
    std::string zoneID;

public:
  void serialize_impl() override;
  void deserialize_impl() override;

public:
    SelfAuthSpaceInfo() = default;
};

//subAuthSearch
class AuthSearch: public gvds::JsonSerializer {
public:
  std::string gvdsID;
  std::vector<std::string> vec_ZoneID;
  std::map<std::string, std::string> read;
  std::map<std::string, std::string> write;
  std::map<std::string, std::string> exe;
  std::map<std::string, std::string> isowner;

  std::map<std::string, std::string> ownergroupR;
  std::map<std::string, std::string> ownergroupW;
  std::map<std::string, std::string> ownergroupE;

  std::map<std::string, std::string> zoneName;
public:
  void serialize_impl() override;
  void deserialize_impl() override;

public:
  AuthSearch() = default;
};

//modift group auth
class AuthModifygroupinfo: public gvds::JsonSerializer {
  public:
  std::string spaceinformation;
  std::string gvdsID;
  std::string au_person;
  std::string au_group;
  std::string au_other;
public:
  void serialize_impl() override;
  void deserialize_impl() override;
public:
  AuthModifygroupinfo() = default;
};


class FEAuthModifygroupinfo: public gvds::JsonSerializer {
  public:
  std::string gvdsID;
  std::string zonename;
  std::string modify_groupauth;
public:
  void serialize_impl() override;
  void deserialize_impl() override;
public:
  FEAuthModifygroupinfo() = default;
};


class SelfSPD: public gvds::JsonSerializer {
public:
  std::string spaceinformation;
  std::string gp; //组名字，就是 zoneID.substr(0,9)
  std::string zoneID;
  std::string spaceID;
public:
  void serialize_impl() override;
  void deserialize_impl() override;
public:
  SelfSPD() = default;
};

/*
class Auth: public gvds::JsonSerializer {
public:
    std::string GVDSAccountID;
    std::vector<std::string> zoneid;
    std::map<std::string, int> authwrite;
    std::map<std::string, int> authread;
    std::map<std::string, int> authexe;
    std::map<std::string, int> authadd_del;
    std::map<std::string, int> authass_p;

public:
  void serialize_impl() override;
  void deserialize_impl() override;

public:
    Auth() = default;
    Auth(std::string id) : GVDSAccountID(id){}
};
*/


#endif