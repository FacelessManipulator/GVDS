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
using namespace std;
using namespace libconfig;




// 创建配置文件解析模块
ConfigureWrapper::ConfigureWrapper(const char* configfile) {
    try {
        baseCfg.setOptions(Config::OptionFsync
                       | Config::OptionSemicolonSeparators
                       | Config::OptionColonAssignmentForGroups
                       | Config::OptionOpenBraceOnSeparateLine);
        baseCfg.readFile(configfile);
    }catch (const FileIOException &fileIOException){
        cerr<< "文件 I/O 错误" << endl;
    }catch (const ParseException &parseException){
        cerr << "文件解析错误： " << parseException.getFile() << ":" << parseException.getLine()<< " - " << parseException.getError() << endl;
    }
}

ConfigureWrapper::~ConfigureWrapper() {

}

Setting& ConfigureWrapper::getRoot() {
    return baseCfg.getRoot();
}

Setting& ConfigureWrapper::lookUpByPath(const char* path) {
    try {
        return  baseCfg.lookup(path);
    }catch (const SettingNotFoundException &settingNotFoundException) {
        cerr << "未发现此配置项目." << endl;
    }
}

bool ConfigureWrapper::writeFile(const char *outfile) {
    try
    {
        baseCfg.writeFile(outfile);
        cerr << "配置文件更新成功: " << outfile << endl;
        return true;
    }
    catch(const FileIOException &fileIOException)
    {
        cerr << "文件写入过程 I/O 错误" << outfile << endl;
        return false;
    }
}
