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