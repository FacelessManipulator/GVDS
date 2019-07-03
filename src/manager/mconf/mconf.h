
#ifndef MCONF_H
#define MCONF_H

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <iostream>
#include <map>


#include "datastore/couchbase_helper.h"
#include "manager/manager.h"
#include "common/centerinfo.h"



//using namespace Pistache;

namespace hvs{
class Mconf : public ManagerModule {

private:
  virtual void start() override;
  virtual void stop() override;
  virtual void router(Pistache::Rest::Router&) override;

public:
 //--------------------------------------------
    //define your function here
    //添加和修改都是此接口
    void addCenterRest(const Rest::Request& request, Http::ResponseWriter response);
    int addCenter(FECenterInfo &FEcenter);

    //查询
    void searchCenterRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string searchCenter();

    //删除
    void deleteCenterRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string deleteCenter(std::string centerID);


public:
    Mconf() : ManagerModule("mconf") {
      auto _config = HvsContext::get_context()->_config;
      bucket_account_info = _config->get<std::string>("bucket.account_info").value_or("account_info");
      
      key = "center_information";
    };
    ~Mconf() {};

private:
  string key;
  string bucket_account_info;
};

}//namespace

#endif