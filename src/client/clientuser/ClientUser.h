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

    void getAccountID(std::string m);
    std::string setAccountID();



private:
    bool m_stop;

    std::string mtoken;
    std::string accountName;
    std::string accountID;

};

}//namespace

#endif