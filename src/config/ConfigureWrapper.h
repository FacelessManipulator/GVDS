/**
 * Created by Yaowen Xu on 2019-02-26.
 * 作者: Yaowen Xu
 * 时间: 2019-02-26
 * 工程: Configlib
 * 作者单位: 北京航空航天大学计算机学院-系统结构研究所
 */

#ifndef CONFIGLIB_CONFIGUREWRAPPER_H
#define CONFIGLIB_CONFIGUREWRAPPER_H

//#include <iostream>
#include <string>
#include <libconfig.h++>
using namespace std;
using namespace libconfig;

class ConfigureWrapper {
public:

    ConfigureWrapper(const char* configfile);
    ~ConfigureWrapper();
    Setting& lookUpByPath(const char* path);
    Setting& getRoot();
    bool writeFile(const char *outfile);
protected:

private:
    Config baseCfg; // 配置文件整体
};


#endif //CONFIGLIB_CONFIGUREWRAPPER_H
