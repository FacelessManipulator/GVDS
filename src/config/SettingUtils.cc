/**
 * Created by Yaowen Xu on 2019-02-27.
 * 作者: Yaowen Xu
 * 时间: 2019-02-27
 * 工程: Configlib
 * 作者单位: 北京航空航天大学计算机学院-系统结构研究所
 */

#include "SettingUtils.h"


string SettingUtils::getStringValue(const Setting &setting, const char *name) {
    string value;
    setting.lookupValue(name, value);
    return value;
}

int SettingUtils::getIntValue(const Setting &setting, const char *name) {
    int value;
    setting.lookupValue(name, value);
    return value;
}

double SettingUtils::getDoubleValue(const Setting &setting, const char *name) {
    double value;
    setting.lookupValue(name, value);
    return value;
}

bool SettingUtils::format(const char* inpath, const char* outpath) {
    Config cfg;
    cfg.setOptions(Config::OptionFsync
                   | Config::OptionSemicolonSeparators
                   | Config::OptionColonAssignmentForGroups
                   | Config::OptionOpenBraceOnSeparateLine);
    try
    {
        cfg.readFile(inpath);
    }
    catch(const FileIOException &fioex)
    {
        std::cerr << "读取文件时发生 I/O 错误." << std::endl;
        return false;
    }
    catch(const ParseException &pex)
    {
        std::cerr << "文件解析错误： " << pex.getFile() << ":" << pex.getLine()
                  << " - " << pex.getError() << std::endl;
        return false;
    }

    try
    {
        cfg.writeFile(outpath);
        cerr << "格式化文件成功: " << outpath << endl;
    }
    catch(const FileIOException &fileIOException)
    {
        cerr << "文件写入过程 I/O 错误" << outpath << endl;
        return false;
    }

    return true;
}
