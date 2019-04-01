/**
 * Created by Yaowen Xu on 2019-02-26.
 * 作者: Yaowen Xu
 * 时间: 2019-02-26
 * 工程: Configlib
 * 作者单位: 北京航空航天大学计算机学院-系统结构研究所
 */

#include "ConfigureWrapper.h"
#include <iostream>
#include <libconfig.h++>
using namespace libconfig;

// 创建配置文件解析模块
ConfigureWrapper::ConfigureWrapper(const char* configfile) {
    try {
//        baseCfg.setOptions(Config::OptionFsync
//                       | Config::OptionSemicolonSeparators
//                       | Config::OptionColonAssignmentForGroups
//                       | Config::OptionOpenBraceOnSeparateLine);
        baseCfg.readFile(configfile);
    }catch (const FileIOException &fileIOException){
        std::cerr<< "文件 I/O 错误" << std::endl;
    }catch (const ParseException &parseException){
        std::cerr << "文件解析错误： " << parseException.getFile() << ":" << parseException.getLine()<< " - " << parseException.getError() << std::endl;
    }
}

ConfigureWrapper::~ConfigureWrapper() = default;

Setting& ConfigureWrapper::getRoot() {
    return baseCfg.getRoot();
}

Setting& ConfigureWrapper::lookUpByPath(const char* path) {
    try {
        return  baseCfg.lookup(path);
    }catch (const SettingNotFoundException &settingNotFoundException) {
        std::cerr << "未发现此配置项目." << std::endl;
    }
}

bool ConfigureWrapper::writeFile(const char *outfile) {
    try{
        baseCfg.writeFile(outfile);
        std::cerr << "配置文件更新成功: " << outfile << std::endl;
        return true;
    }catch(const FileIOException &fileIOException){
        std::cerr << "文件写入过程 I/O 错误" << outfile << std::endl;
        return false;
    }
}
