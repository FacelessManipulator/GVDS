#include <iostream>
#include <pthread.h>
#include <vector>
#include <unistd.h>
#include <unistd.h>
#include <utility> //pair
#include <algorithm>  //sort
#include "common/JsonSerializer.h"
#include "common/json.h"

#include <pistache/client.h>
#include <pistache/http.h>
#include <pistache/net.h>
#include <atomic>

#include "ClientUser.h"
#include "client/client.h"
#include "client/ipc_mod.h"
#include "client/msg_mod.h"



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

void ClientUser::setAccountID(std::string m){
    this->accountID = m;
}
std::string ClientUser::getAccountID(){
    return this->accountID;
}


    //输入member的vector<string> name , 返回一个vector<string> id;
bool ClientUser::getMemberID(std::vector<std::string> memberName, std::vector<std::string> &memberID){
    //序列化
    std::string json_str = json_encode(memberName);

    string endpoint = client->get_manager();
    string res = client->rpc->post_request(endpoint, "/users/memberID", json_str);

    if(res=="fail"){
        cout << "getMemberID fail"<< endl;
        return false;
    }

    //反序列化res
    json_decode(res, memberID);
    return true;
    // center_Information = response;
}

}//namespace