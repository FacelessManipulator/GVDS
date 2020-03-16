#include "client/clientuser/ClientAuth_struct.h"
#include <iostream>
#include "common/JsonSerializer.h"
#include "datastore/datastore.h"
#include "context.h"


void Auth::serialize_impl() {

    put("zoneID", zoneID);
    put("GVDSAccountID", GVDSAccountID);
    put("owner_read", owner_read);
    put("owner_write", owner_write);
    put("owner_exe", owner_exe);

    put("group_read", group_read);
    put("group_write", group_write);
    put("group_exe", group_exe);

    put("other_read", other_read);
    put("other_write", other_write);
    put("other_exe", other_exe);
}

void Auth::deserialize_impl() {

    get("zoneID", zoneID);
    get("GVDSAccountID", GVDSAccountID);
    get("owner_read", owner_read);
    get("owner_write", owner_write);
    get("owner_exe", owner_exe);

    get("group_read", group_read);
    get("group_write", group_write);
    get("group_exe", group_exe);

    get("other_read", other_read);
    get("other_write", other_write);
    get("other_exe", other_exe);
}
//
void SelfAuthSpaceInfo::serialize_impl() {
    put("spaceinformation", spaceinformation);
    put("ownerID_zone", ownerID_zone);
    put("memberID", memberID);
    put("zoneID", zoneID);
}

void SelfAuthSpaceInfo::deserialize_impl() {
    get("spaceinformation", spaceinformation);
    get("ownerID_zone", ownerID_zone);
    get("memberID", memberID);
    get("zoneID", zoneID);
}
//
void AuthSearch::serialize_impl(){
    put("gvdsID", gvdsID);
    put("vec_ZoneID", vec_ZoneID);
    put("read", read);
    put("write", write);
    put("exe", exe);
    put("isowner", isowner);
    put("ownergroupR", ownergroupR);
    put("ownergroupW", ownergroupW);
    put("ownergroupE", ownergroupE);
    put("zoneName", zoneName);
}

void AuthSearch::deserialize_impl(){
    get("gvdsID", gvdsID);
    get("vec_ZoneID", vec_ZoneID);
    get("read", read);
    get("write", write);
    get("exe", exe);
    get("isowner", isowner);
    get("ownergroupR", ownergroupR);
    get("ownergroupW", ownergroupW);
    get("ownergroupE", ownergroupE);
    get("zoneName", zoneName);
}
//

void AuthModifygroupinfo::serialize_impl(){
    put("spaceinformation", spaceinformation);
    put("gvdsID", gvdsID);
    put("au_person", au_person);
    put("au_group", au_group);
    put("au_other", au_other);
}

void AuthModifygroupinfo::deserialize_impl(){
    get("spaceinformation", spaceinformation);
    get("gvdsID", gvdsID);
    get("au_person", au_person);
    get("au_group", au_group);
    get("au_other", au_other);
}


//
void FEAuthModifygroupinfo::serialize_impl(){
    put("gvdsID", gvdsID);
    put("zonename", zonename);
    put("modify_groupauth", modify_groupauth);
}

void FEAuthModifygroupinfo::deserialize_impl(){
    get("gvdsID", gvdsID);
    get("zonename", zonename);
    get("modify_groupauth", modify_groupauth);
}

//
void SelfSPD::serialize_impl(){
    put("spaceinformation", spaceinformation);
    put("gp", gp);
    put("zoneID", zoneID);
    put("spaceID", spaceID);
}

void SelfSPD::deserialize_impl(){
    get("spaceinformation", spaceinformation);
    get("gp", gp);
    get("zoneID", zoneID);
    get("spaceID", spaceID);
}

