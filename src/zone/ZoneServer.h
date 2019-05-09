#ifndef ZONESERVER_H
#define ZONESERVER_H

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <iostream>
#include <map>


#include "datastore/couchbase_helper.h"
#include "zone/Zone.h"
#include "space/SpaceServer.h"
#include "zone/Zone.h"
//class Account;//没懂

using namespace Pistache;


namespace hvs{
class ZoneServer {
public:
    static ZoneServer* getInstance(){
        if (instance == nullptr)
            instance = new ZoneServer();
	    return instance;
    };
 //--------------------------------------------
    //define your function here
    
    //区域重命名模块：区域重命名接口
    void ZoneRenameRest(const Rest::Request& request, Http::ResponseWriter response);
    int ZoneRename(std::string zoneID, std::string ownerID, std::string newZoneName);

    //区域定位模块：区域定位接口
    void GetZoneLocateInfoRest(const Rest::Request& request, Http::ResponseWriter response);
    bool GetZoneLocateInfo(std::vector<std::string> &result, std::string clientID, std::string zoneID, std::vector<std::string> spaceID);

    //区域信息检索模块：区域信息检索接口
    void GetZoneInfoRest(const Rest::Request& request, Http::ResponseWriter response);
    bool GetZoneInfo(std::vector<std::string> &result_z, std::string clientID);

    //区域共享模块：区域共享接口
    void ZoneShareRest(const Rest::Request& request, Http::ResponseWriter response);
    int ZoneShare(std::string zoneID, std::string ownerID, std::vector<std::string> memberID);

    //区域共享模块：区域共享取消接口
    void ZoneShareCancelRest(const Rest::Request& request, Http::ResponseWriter response);
    int ZoneShareCancel(std::string zoneID, std::string ownerID, std::vector<std::string> memberID);

    //区域注册模块：区域注册接口
    void ZoneRegisterRest(const Rest::Request& request, Http::ResponseWriter response);
    int ZoneRegister(std::string zoneName, std::string ownerID, std::vector<std::string> memberID,
                     std::string spaceName, int64_t spaceSize, std::string spacePathInfo);

    //区域注销模块：区域注销接口
    void ZoneCancelRest(const Rest::Request& request, Http::ResponseWriter response);
    int ZoneCancel(std::string zoneID, std::string ownerID);




 //--------------------------------------------
private:
    ZoneServer() = default;
    ~ZoneServer();

    static ZoneServer* instance;  //single object
};

//std::string md5(std::string strPlain);


}// namespace hvs

//判断vector1是否包含vector2
bool isSubset(std::vector<std::string> v1, std::vector<std::string> v2);

#endif







