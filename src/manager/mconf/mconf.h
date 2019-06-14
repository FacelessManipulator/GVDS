
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
    void addCenterRest(const Rest::Request& request, Http::ResponseWriter response);
    int addCenter(FECenterInfo &FEcenter);

    void searchCenterRest(const Rest::Request& request, Http::ResponseWriter response);
    std::string searchCenter();

public:
    Mconf() : ManagerModule("mconf") {
        key = "center_information";
    };
    ~Mconf() {};

private:
  string key;
};

}//namespace

#endif