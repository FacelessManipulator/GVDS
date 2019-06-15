#include <iostream>
#include <pthread.h>
#include <vector>
#include <unistd.h>
#include <unistd.h>
#include <utility> //pair
#include <algorithm>  //sort

#include <pistache/client.h>
#include <pistache/http.h>
#include <pistache/net.h>
#include <atomic>

#include "ClientUser.h"


using namespace Pistache;
using namespace Pistache::Http;
using namespace hvs;
using namespace std;

//mutex mtx_write

namespace hvs{

void ClientUser::start(){
    m_stop = false;
    //create("opt_node");
}
void ClientUser::stop(){
    m_stop = true;
}

void ClientUser::setToken(std::string m){
    this->mtoken = m;
}
std::string ClientUser::getToken(){
    return this->mtoken;
}


void ClientUser::setAccountName(std::string m){
    this->accountName = m;
}
std::string ClientUser::getAccountName(){
    return this->accountName;
}

void ClientUser::getAccountID(std::string m){
    this->accountID = m;
}
std::string ClientUser::setAccountID(){
    return this->accountID;
}
}//namespace