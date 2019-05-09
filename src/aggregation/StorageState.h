#pragma once

/***********************************************************
 * @file  StorageState.h
 * @brief 资源状态类
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


/// 资源状态类，标识资源的状态
/**
 * @author: 韦冰
 * @date: 2018/3/25
 *
 * 
 */
namespace hvs {
enum StorageResState// 存储资源状态
{
    Initializing = 0,// 初始化中
    Normal,// 正常使用
    OverLoad,// 负载过载
    Logouting,// 注销退出中
    Quited// 已经退出
};
}// namespace hvs