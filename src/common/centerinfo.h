#ifndef CENTERINFO_H
#define CENTERINFO_H

#include "common/JsonSerializer.h"
#include <map>  

class CenterInfo : public gvds::JsonSerializer {
public:
    std::vector<std::string> centerID;
    std::map<std::string, std::string> centerIP;
    std::map<std::string, std::string> centerPort;
    std::map<std::string, std::string> centerName;

public:
  void serialize_impl() override{
        put("centerID", centerID);
        put("centerIP", centerIP);
        put("centerPort", centerPort);
        put("centerName", centerName);

  }
  void deserialize_impl() override{
        get("centerID", centerID);
        get("centerIP", centerIP);
        get("centerPort", centerPort);
        get("centerName", centerName);
  }

public:
    CenterInfo() = default;
};


class FECenterInfo : public gvds::JsonSerializer {
public:
    std::string centerID;
    std::string centerIP;
    std::string centerPort;
    std::string centerName;

public:
  void serialize_impl() override{
        put("centerID", centerID);
        put("centerIP", centerIP);
        put("centerPort", centerPort);
        put("centerName", centerName);

  }
  void deserialize_impl() override{
        get("centerID", centerID);
        get("centerIP", centerIP);
        get("centerPort", centerPort);
        get("centerName", centerName);
  }

public:
    FECenterInfo() = default;
};

#endif