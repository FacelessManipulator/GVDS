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

class ConfigureWrapper {
public:
    explicit ConfigureWrapper(const char* configfile);
    ~ConfigureWrapper();
    libconfig::Setting& lookUpByPath(const char* path);
    libconfig::Setting& getRoot();
    bool writeFile(const char *outfile);

private:

    libconfig::Config baseCfg; // 整体配置文件
};

#endif //CONFIGLIB_CONFIGUREWRAPPER_H
