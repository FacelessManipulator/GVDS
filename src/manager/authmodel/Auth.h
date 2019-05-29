#ifndef AUTH_H
#define AUTH_H

#include "common/JsonSerializer.h"
#include "datastore/datastore.h"
#include "context.h"
#include <map>  

class Auth: public hvs::JsonSerializer {
public:
    std::string zoneID;
    std::string HVSAccountID;
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
class SelfAuthSpaceInfo: public hvs::JsonSerializer {
public:
    std::string spaceinformation;
    std::string ownerID_zone;
    std::vector<std::string> memberID;

public:
  void serialize_impl() override;
  void deserialize_impl() override;

public:
    SelfAuthSpaceInfo() = default;
};

/*
class Auth: public hvs::JsonSerializer {
public:
    std::string HVSAccountID;
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
    Auth(std::string id) : HVSAccountID(id){}
};
*/


#endif