#include "client/clientuser/ClientUser_struct.h"   // 只有这块和服务端不一样 Account.cc
#include <iostream>
#include "common/JsonSerializer.h"
#include "datastore/datastore.h"
#include "context.h"


/*
int lbqprint(){
    std::cout<<"lbqprint"<<std::endl;
    return 0;
}
*/


void Account::serialize_impl() {

    put("GVDSAccountName", accountName);
    put("GVDSPassword", Password);
    put("GVDSAccountID", accountID);
    put("AccountEMAIL", accountEmail);
    put("AccountPHONE", accountPhone);
    put("AccountAddress", accountAddress);
    put("Department", Department);
    //put("SC", sc);

}
void Account::deserialize_impl() {

    get("GVDSAccountName", accountName);
    get("GVDSPassword", Password);
    get("GVDSAccountID", accountID);
    get("AccountEMAIL", accountEmail);
    get("AccountPHONE", accountPhone);
    get("AccountAddress", accountAddress);
    get("Department", Department);
    //get("SC", sc);
 }



void SCAccount::serialize_impl() {
    put("accountID", accountID);
    put("centerName", centerName);
    put("localaccount", localaccount);
    put("localpassword", localpassword);
}
void SCAccount::deserialize_impl() {
    get("accountID", accountID);
    get("centerName", centerName);
    get("localaccount", localaccount);
    get("localpassword", localpassword);
 }


void AccountPair::serialize_impl() {
    put("GVDSAccountName", accountName);
    put("GVDSAccountID", accountID);
}

void AccountPair::deserialize_impl() {
    get("GVDSAccountName", accountName);
    get("GVDSAccountID", accountID);
 }
 

 void AccountPass::serialize_impl() {
    put("GVDSAccountName", accountName);
    put("GVDSPassword", Password);
}

void AccountPass::deserialize_impl() {
    get("GVDSAccountName", accountName);
    get("GVDSPassword", Password);
 }
 


void AccountSCPool::serialize_impl(){
    put("unuse_account", unuse_account);
    put("use_account", use_account);
}

void AccountSCPool::deserialize_impl(){
    get("unuse_account", unuse_account);
    get("use_account", use_account);
}


void LocalAccountPair::serialize_impl(){
    put("localaccount", localaccount);
    put("localpassword", localpassword);
}

void LocalAccountPair::deserialize_impl(){
    get("localaccount", localaccount);
    get("localpassword", localpassword);
}


//admin
void struct_AdminList::serialize_impl(){
    put("namelist", namelist);
}

void struct_AdminList::deserialize_impl(){
    get("namelist", namelist);
}

void struct_AdminAccountMap::serialize_impl(){
    put("adgvdsID", adgvdsID);
    put("gvdsID", gvdsID);
    put("hostCenterName", hostCenterName);
}

void struct_AdminAccountMap::deserialize_impl(){
    get("adgvdsID", adgvdsID);
    get("gvdsID", gvdsID);
    get("hostCenterName", hostCenterName);
}



// void struct_apply_info::serialize_impl(){
//     put("id", id);
//     put("data", data);
// }

// void struct_apply_info::deserialize_impl(){
//     get("id", id);
//     get("data", data);
// }