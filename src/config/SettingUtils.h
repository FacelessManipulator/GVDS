/**
 * Created by Yaowen Xu on 2019-02-27.
 * 作者: Yaowen Xu
 * 时间: 2019-02-27
 * 工程: Configlib
 * 作者单位: 北京航空航天大学计算机学院-系统结构研究所
 */

#ifndef CONFIGLIB_SETTINGUTILS_H
#define CONFIGLIB_SETTINGUTILS_H
#include <string>
#include <libconfig.h++>
#include <iostream>
using namespace std;
using namespace libconfig;

class SettingUtils {
public:
    static string getStringValue(const Setting& setting, const char* name);
    static int getIntValue(const Setting& setting, const char* name);
    static double getDoubleValue(const Setting& setting, const char* name);
    static bool format(const char* inpath, const char* outpath);
};


#endif //CONFIGLIB_SETTINGUTILS_H
