#pragma once

/***********************************************************
 * @file  AggregationServer.h
 * @brief 存储资源聚合服务类
 * @author 韦冰
 * @version 0.0.1
 * @date 2018/3/25
 * @email weibing@buaa.edu.cn
 * @license GNU General Public License (GPL)
 *
 * 修改历史：
 * ----------------------------------------------
 * 日期     | 版本号  |   作者   |      描述
 * ----------------------------------------------
 * 2018/3/25 | 0.0.1  | 韦冰   | 实现基础功能
 * ----------------------------------------------
 *
 *
 ***********************************************************/

#include "StorageResBicInfo.h"
#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <string>

/// 存储资源聚合服务类
/**
 * @author: 韦冰
 * @date: 2018/3/25
 *
 * 存储资源聚合服务类。
 */
namespace hvs {
class AggregationServer {
 
public:
    static AggregationServer* getInstance(){
        if (instance == nullptr)
            instance = new AggregationServer();
	    return instance;
    };


    void StorageResRegisterRest(const Rest::Request& request, Http::ResponseWriter response);
    void StorageResLogoutRest(const Rest::Request& request, Http::ResponseWriter response);

 //--------------------------------------------
private:
    AggregationServer();
    virtual ~AggregationServer();

    static AggregationServer* instance;  //single object

};
}// namespace hvs