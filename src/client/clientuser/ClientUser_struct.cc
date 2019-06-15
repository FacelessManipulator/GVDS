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

    put("HVSAccountName", accountName);
    put("HVSPassword", Password);
    put("HVSAccountID", accountID);
    put("AccountEMAIL", accountEmail);
    put("AccountPHONE", accountPhone);
    put("AccountAddress", accountAddress);
    put("Department", Department);
    //put("SC", sc);

}
void Account::deserialize_impl() {

    get("HVSAccountName", accountName);
    get("HVSPassword", Password);
    get("HVSAccountID", accountID);
    get("AccountEMAIL", accountEmail);
    get("AccountPHONE", accountPhone);
    get("AccountAddress", accountAddress);
    get("Department", Department);
    //get("SC", sc);
 }



void SCAccount::serialize_impl() {
    put("HVSAccountID", accountID);
    put("Beijing_account", Beijing_account);
    put("Shanghai_account", Shanghai_account);
    put("Guangzhou_account", Guangzhou_account);
    put("Changsha_account", Changsha_account);
    put("Jinan_account", Jinan_account);

}
void SCAccount::deserialize_impl() {
    get("HVSAccountID", accountID);
    get("Beijing_account", Beijing_account);
    get("Shanghai_account", Shanghai_account);
    get("Guangzhou_account", Guangzhou_account);
    get("Changsha_account", Changsha_account);
    get("Jinan_account", Jinan_account);
 }


void AccountPair::serialize_impl() {
    put("HVSAccountName", accountName);
    put("HVSAccountID", accountID);
}

void AccountPair::deserialize_impl() {
    get("HVSAccountName", accountName);
    get("HVSAccountID", accountID);
 }
 

 void AccountPass::serialize_impl() {
    put("HVSAccountName", accountName);
    put("HVSPassword", Password);
}

void AccountPass::deserialize_impl() {
    get("HVSAccountName", accountName);
    get("HVSPassword", Password);
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