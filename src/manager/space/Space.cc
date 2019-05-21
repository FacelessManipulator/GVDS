#include "manager/space/Space.h" 
#include <iostream>
#include "common/JsonSerializer.h"
#include "datastore/datastore.h"
#include "Space.h"

//#include "context.h"

void Space::serialize_impl() {
    put("UUID", spaceID);
    put("name", spaceName);
    put("capcity", spaceSize);
    put("SC_UUID", hostCenterID);
    put("Storage_UUID", storageSrcID);
    put("root_location", spacePath);
    put("Status", status);
}
void Space::deserialize_impl() {
    get("UUID", spaceID);
    get("name", spaceName);
    get("capcity", spaceSize);
    get("SC_UUID", hostCenterID);
    get("Storage_UUID", storageSrcID);
    get("root_location", spacePath);
    get("Status", status);
}


void SpaceInfo::serialize_impl() {
    put("UUID", spaceID);
    put("name", spaceName);
    put("capcity", spaceSize);
}

void SpaceInfo::deserialize_impl() {
    get("UUID", spaceID);
    get("name", spaceName);
    get("capcity", spaceSize);
}


void SpaceMetaData::serialize_impl() {
    put("UUID", spaceID);
    put("name", spaceName);
    put("storagesrcid", storageSrcID);
    put("storagesrcname", storageSrcName);
    put("hostcenterid", hostCenterID);
    put("hostcentername", hostCenterName);
    put("spacepath", spacePath);
}
void SpaceMetaData::deserialize_impl() {
    get("UUID", spaceID);
    get("name", spaceName);
    get("storagesrcid", storageSrcID);
    get("storagesrcname", storageSrcName);
    get("hostcenterid", hostCenterID);
    get("hostcentername", hostCenterName);
    get("spacepath", spacePath);
}

void SpaceRenameReq::serialize_impl()   {
    put("UUID", spaceID);
    put("newname", newSpaceName);
}

void SpaceRenameReq::deserialize_impl()   {
    get("UUID", spaceID);
    get("newname", newSpaceName);
}

void SpaceSizeChangeReq::serialize_impl() {
    put("UUID", spaceID);
    put("name", spaceName);
    put("capcity", newSpaceSize);
}

void SpaceSizeChangeReq::deserialize_impl() {
    get("UUID", spaceID);
    get("name", spaceName);
    get("capcity", newSpaceSize);
}
