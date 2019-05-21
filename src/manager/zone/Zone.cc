#include "manager/zone/Zone.h"
#include <iostream>
#include "common/JsonSerializer.h"
#include "datastore/datastore.h"
#include "context.h"

void Zone::serialize_impl()   {
    put("UUID", zoneID);
    put("name", zoneName);
    put("owner", ownerID);
    put("members", memberID);
    put("spaces", spaceID);
}

void Zone::deserialize_impl()   {
    get("UUID", zoneID);
    get("name", zoneName);
    get("owner", ownerID);
    get("members", memberID);
    get("spaces", spaceID);
}


void ZoneInfo::serialize_impl()   {
    put("UUID", zoneID);
    put("name", zoneName);
    put("owner", ownerID);
    put("members", memberID);
    put("spaceinfo", spaceBicInfo);
}

void ZoneInfo::deserialize_impl()   {
    get("UUID", zoneID);
    get("name", zoneName);
    get("owner", ownerID);
    get("members", memberID);
    get("spaceinfo", spaceBicInfo);
}

void ZoneRenameReq::serialize_impl()   {
    put("UUID", zoneID);
    put("owner", ownerID);
    put("newname", newZoneName);
}

void ZoneRenameReq::deserialize_impl()   {
    get("UUID", zoneID);
    get("owner", ownerID);
    get("newname", newZoneName);
}

void GetZoneLocateInfoReq::serialize_impl()   {
    put("client", clientID);
    put("zone", zoneID);
    put("spaces", spaceID);
}

void GetZoneLocateInfoReq::deserialize_impl()   {
    get("client", clientID);
    get("zone", zoneID);
    get("spaces", spaceID);
}

void GetZoneLocateInfoRes::serialize_impl()   {
    put("ZoneLocateInfo", zoneLocateInfoResult);
}

void GetZoneLocateInfoRes::deserialize_impl()   {
    get("ZoneLocateInfo", zoneLocateInfoResult);
}

void GetZoneInfoRes::serialize_impl()   {
    put("ZoneInfo", zoneInfoResult);
}

void GetZoneInfoRes::deserialize_impl()   {
    get("ZoneInfo", zoneInfoResult);
}

void ZoneShareReq::serialize_impl()   {
    put("UUID", zoneID);
    put("owner", ownerID);
    put("members", memberID);
}

void ZoneShareReq::deserialize_impl()   {
    get("UUID", zoneID);
    get("owner", ownerID);
    get("members", memberID);
}

void ZoneRegisterReq::serialize_impl()   {
    put("name", zoneName);
    put("owner", ownerID);
    put("members", memberID);
    put("spa_name", spaceName);
    put("size", spaceSize);
    put("path", spacePathInfo);
    //put("managenode", globalManageNodeInfo);
}

void ZoneRegisterReq::deserialize_impl()   {
    get("name", zoneName);
    get("owner", ownerID);
    get("members", memberID);
    get("spa_name", spaceName);
    get("size", spaceSize);
    get("path", spacePathInfo);
    //get("managenode", globalManageNodeInfo);
}

void ZoneCancelReq::serialize_impl()   {
    put("UUID", zoneID);
    put("owner", ownerID);
}

void ZoneCancelReq::deserialize_impl()   {
    get("UUID", zoneID);
    get("owner", ownerID);
}

void MapAddReq::serialize_impl()   {
    put("UUID", zoneID);
    put("owner", ownerID);
    put("spa_name", spaceName);
    put("size", spaceSize);
    put("path", spacePathInfo);
    //put("managenode", globalManageNodeInfo);
}

void MapAddReq::deserialize_impl()   {
    get("UUID", zoneID);
    get("owner", ownerID);
    get("spa_name", spaceName);
    get("size", spaceSize);
    get("path", spacePathInfo);
    //get("managenode", globalManageNodeInfo);
}

void MapDeductReq::serialize_impl()   {
    put("UUID", zoneID);
    put("owner", ownerID);
    put("spaces", spaceID);
}

void MapDeductReq::deserialize_impl()   {
    get("UUID", zoneID);
    get("owner", ownerID);
    get("spaces", spaceID);
}