//
// Created by yaowen on 5/25/19.
// 北航系统结构所-存储组
//

#include "gtest/gtest.h"
#include "manager/space/SpaceServer.h"

TEST(SPACEFUNC, create){
    hvs::init_context();
    std::string rootdir = *(hvs::HvsContext::get_context()->_config->get<std::string>("storage"));
    rootdir += "superman";
    int mkret = mkdir(rootdir.c_str(), 0777);
    if (mkret !=0 ) {
        perror("SpaceCreate 文件夹创建错误！");
    }
}