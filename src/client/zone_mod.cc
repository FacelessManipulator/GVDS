//
// Created by yaowen on 5/28/19.
// 北航系统结构所-存储组
//

#include "zone_mod.h"

void hvs::ClientZone::start(){
    std::cerr << "空间模块启动!" << std::endl;
    GetZoneInfo(ownID);
}


void hvs::ClientZone::stop(){}

std::vector<std::string> hvs::ClientZone::GetZoneInfo(std::string clientID) {
    std::cout << "ZoneInfo Called!" << std::endl; // 调用获取区域信息；
    return std::vector<std::string>();
}
