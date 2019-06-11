#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "common/JsonSerializer.h"
#include "datastore/datastore.h"
#include "context.h"
#include <map>  


//对应sc_account_info表
class SCAccount : public hvs::JsonSerializer {
public:
    std::string accountID;
    std::map<std::string, std::string> Beijing_account;
    std::map<std::string, std::string> Shanghai_account;
    std::map<std::string, std::string> Guangzhou_account;
    std::map<std::string, std::string> Changsha_account;
    std::map<std::string, std::string> Jinan_account;


public:
  void serialize_impl() override;
  void deserialize_impl() override;

public:
    SCAccount() = default;
    SCAccount(std::string id) : accountID(id){}
};


//对应account_info表
class Account : public hvs::JsonSerializer {
public:

    std::string accountName; //虚拟数据空间账户名
    std::string Password;//虚拟数据空间账户密码
    std::string accountID;//虚拟数据空间账户ID,在客户端生成
    std::string accountEmail;//虚拟数据空间用户邮箱
    std::string accountPhone;//虚拟数据空间用户电话
    std::string accountAddress;//虚拟数据空间用户住址
    std::string Department;//单位名称
    //SCAccount sc;

 public:
  void serialize_impl() override;
  void deserialize_impl() override;

 public:
  Account() = default;
  Account(std::string name, std::string pass, std::string AID, 
          std::string email, std::string phone,std::string address, std::string dep)
           : accountName(name), Password(pass), accountID(AID), accountEmail(email), accountPhone(phone), accountAddress(address), Department(dep){}
};

//账户注册时写入，账户登录时查询，对应account_map_id 表
class AccountPair : public hvs::JsonSerializer{
public:
    std::string accountName; //虚拟数据空间账户名
    std::string accountID;   //虚拟数据空间账户ID,在客户端生成

public:
    void serialize_impl() override;
    void deserialize_impl() override;

public:
    AccountPair() = default;
    AccountPair(std::string name, std::string id)
     : accountName(name), accountID(id){}
};


//账户登录时，解析body的账户名，密码，不对应具体表项
class AccountPass : public hvs::JsonSerializer{
public:
    std::string accountName; //虚拟数据空间账户名
    std::string Password;//虚拟数据空间账户密码
public:
    void serialize_impl() override;
    void deserialize_impl() override;
public:
    AccountPass() = default;
    AccountPass(std::string name, std::string pass)
     : accountName(name), Password(pass){}
};


//登录时获取从账户池获取账户
class AccountSCPool : public hvs::JsonSerializer{
public:
    std::map<std::string, std::string> unuse_account;
    std::map<std::string, std::string> use_account;
public:
    void serialize_impl() override;
    void deserialize_impl() override;
public:
    AccountSCPool() = default;
};

//getLocalAccountinfo用到,本地账户名，和密码// 不用和数据库交互
class LocalAccountPair : public hvs::JsonSerializer{
public:
    std::string localaccount;
    std::string localpassword;
public:
    void serialize_impl() override;
    void deserialize_impl() override;
public:
    LocalAccountPair() = default;
    LocalAccountPair(std::string account, std::string pass)
    : localaccount(account), localpassword(pass){}
};

//int lbqprint();

/*
class SCAccount : public hvs::JsonSerializer {
 public:

  std::string accountID;//虚拟数据空间账户ID
  std::map<std::string, std::string> SC_account_password;   //zhanghu  password
  std::map<std::string, std::string> SC_account_location;   //zhanghu  password
 

 public:
  void serialize_impl() override {
  


  }
  void deserialize_impl() override {

  }

 public:
  Account() = default;
};

*/

#endif