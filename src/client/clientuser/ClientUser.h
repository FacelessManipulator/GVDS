#ifndef CLIENTUSER_H
#define CLIENTUSER_H

#include <thread>
#include "client/client.h"
#include "common/centerinfo.h"

#include <iostream>
#include <vector>
#include <mutex>




namespace hvs{
class ClientUser: public ClientModule{
private:
    virtual void start() override;
    virtual void stop() override;

    friend class Client;

public:
    ClientUser(const char* name, Client* cli) : ClientModule(name, cli), m_stop(true){
         isThread = true;  
    }
    ~ClientUser(){};

    void setToken(std::string m);
    std::string getToken();

    void setAccountName(std::string m);
    std::string getAccountName();

    void setAccountID(std::string m);
    std::string getAccountID();

    void setApply(std::string id, std::string data);
    std::string getApply(std::string id);
    void clearApply();

    //输入member的vector<string> name , 返回一个vector<string> id;
    bool getMemberID(std::vector<std::string> memberName, std::vector<std::string> &memberID);
 

private:
    bool m_stop;

    std::string mtoken = "1";
    std::string accountName;
    std::string accountID;

    std::map<std::string, std::string> apply_id_data;

};

}//namespace

#endif