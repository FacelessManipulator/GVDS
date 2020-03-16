//
// Created by yaowen on 5/25/19.
// 北航系统结构所-存储组
//

#include "gtest/gtest.h"
#include "manager/space/SpaceServer.h"

bool issSubset(std::vector<std::string> v1, std::vector<std::string> v2)
{
    int i=0,j=0;
    int m=v1.size();
    int n=v2.size();
    if(m<n)
    {
        return 0;
    }
    sort(v1.begin(),v1.end());
    sort(v2.begin(),v2.end());
    while(i<n&&j<m)
    {
        if(v1[j]<v2[i])
        {
            std::cerr << v1[j] << v2[i] << std::endl;
            j++;
        }
        else if(v1[j]==v2[i])
        {
            j++;
            i++;
        }
        else if(v1[j]>v2[i])
        {
            return 0;
        }
    }
    if(i<n)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

TEST(SPACEFUNC, create){
//    gvds::init_context();
//    std::string rootdir = *(gvds::HvsContext::get_context()->_config->get<std::string>("manager.data_path"));
//    rootdir += "superman";
//    int mkret = mkdir(rootdir.c_str(), 0777);
//    if (mkret !=0 ) {
//        perror("SpaceCreate 文件夹创建错误！");
//    }

    std::vector<std::string> v1;
    v1.emplace_back("1");
    v1.emplace_back("2");
    v1.emplace_back("3");
    v1.emplace_back("4");

    std::vector<std::string> v1_1;
    v1.emplace_back("3");
    v1.emplace_back("1");

    std::cerr << issSubset(v1, v1_1) << std::endl;
}