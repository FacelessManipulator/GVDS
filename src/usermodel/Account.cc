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

}
void Account::deserialize_impl() {

    get("HVSAccountName", accountName);
    get("HVSPassword", Password);
    get("HVSAccountID", accountID);
    get("AccountEMAIL", accountEmail);
    get("AccountPHONE", accountPhone);
    get("AccountAddress", accountAddress);
    get("Department", Department);
 }

