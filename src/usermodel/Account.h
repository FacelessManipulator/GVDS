#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "common/JsonSerializer.h"
#include "datastore/datastore.h"
#include "context.h"
#include <map>  


class Account : public hvs::JsonSerializer {
 public:

    std::string accountName; //虚拟数据空间账户名
    std::string Password;//虚拟数据空间账户密码
    std::string accountID;//虚拟数据空间账户ID
    std::string accountEmail;//虚拟数据空间用户邮箱
    std::string accountPhone;//虚拟数据空间用户电话
    std::string accountAddress;//虚拟数据空间用户住址
    std::string Department;//单位名称

 public:
  void serialize_impl() override;
  void deserialize_impl() override;

 public:
  Account() = default;
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