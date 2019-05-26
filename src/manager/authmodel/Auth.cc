#include "manager/authmodel/Auth.h"
#include <iostream>
#include "common/JsonSerializer.h"
#include "datastore/datastore.h"
#include "context.h"


void Auth::serialize_impl() {

    put("zoneID", zoneID);
    put("HVSAccountID", HVSAccountID);
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
    get("HVSAccountID", HVSAccountID);
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
