/**
 * Created by Yaowen Xu on 2019-03-09.
 * 作者: Yaowen Xu
 * 时间: 2019-03-09
 * 工程: GVDS
 * 作者单位: 北京航空航天大学计算机学院-系统结构研究所
 */

#ifndef GVDS_CMDLINEPROXY_H
#define GVDS_CMDLINEPROXY_H

#include <iostream>
#include <boost/program_options.hpp>
#include <cstring>
#include <vector>
#include <memory>
#include <functional>
#include <map> // 创建 map， 进行注册解析函数和执行函数
#include <set>
namespace  po =  boost::program_options;
/*
 * 命令行参数的优先级:
 * 命令行参数 > 代码中指定FLAG > 系统环境变量
 */

class CmdLineProxy {
public:
    CmdLineProxy(int argc, char** argv);
    ~CmdLineProxy();
    void start(); // 配置好解析函数后，就可以开始运行，可以开始运行；
    void test();
    static inline bool is_validate(const std::string& dest) {
        static std::set<char> invalidate{'!','#','@','$','%','^','&','*','~','`','/','|'};
        for(auto i:dest) {
            if(invalidate.count(i)) return false;
        }
        return true;
    }
private:
    void set_global_description();
    void set_command_description();
    void do_global_decode(); // 做全局命令行解析
    void do_module_router();
    void cmdline_print(const char *);
public:
    void print_options();
    int argc; // 命令行参数的个数
    char** argv; // 命令行参数数组
private:
    const char* version = "0.0.1"; // 命令行参数组件版本
    std::string command; // 当前命令的名称
    std::shared_ptr<po::options_description> cmdline_options;
    std::shared_ptr<po::variables_map> sp_variables_map; // 指向当前解析好的类
public:
    std::map<std::string, std::function< void (std::shared_ptr<po::options_description> sp_variables_map)>> cmd_desc_func_map; // 解析函数的映射表
    std::map<std::string, std::function< void (std::shared_ptr<po::variables_map> sp_variables_map)>> cmd_do_func_map; // 解析函数的映射表
};


#endif //GVDS_CMDLINEPROXY_H
