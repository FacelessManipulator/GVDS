#include "usermodel/Account.h"
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
    put("SC", sc);

}
void Account::deserialize_impl() {

    get("HVSAccountName", accountName);
    get("HVSPassword", Password);
    get("HVSAccountID", accountID);
    get("AccountEMAIL", accountEmail);
    get("AccountPHONE", accountPhone);
    get("AccountAddress", accountAddress);
    get("Department", Department);
    get("SC", sc);
 }



void SCAccount::serialize_impl() {
    put("sc_flag", sc_flag);
    put("location_acc", location_scacc);
    put("scaccount_password", scacc_password);

}
void SCAccount::deserialize_impl() {
    get("sc_flag", sc_flag);
    get("location_acc", location_scacc);
    get("scaccount_password", scacc_password);
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
 