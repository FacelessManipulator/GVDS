/**
 * Created by Yaowen Xu on 2019-04-01.
 * 作者: Yaowen Xu
 * 时间: 2019-04-01
 * 工程: HVSONE
 * 作者单位: 北京航空航天大学计算机学院-系统结构研究所
 */

#ifndef HVSONE_CONFIGURESETTINGS_H
#define HVSONE_CONFIGURESETTINGS_H

#include <iostream>
#include <libconfig.h++>
#include <optional>

//包含 libconfig 头文件

class ConfigureSettings {
public:
//    explicit ConfigureSettings(libconfig::Setting& setting):settings(settings){}

//    template <class T>
//    std::optional<T> get_value(const char* name){
//        T value;
//        settings.lookupValue(name, value);
//        return value;
//    }

    // 工具模板函数
    template <class T>
    static T get_value(const libconfig::Setting& setting, const char* name){
        T value;
        setting.lookupValue(name, value);
        return value;
    }

    static bool format(const char* inpath, const char* outpath){
        libconfig::Config cfg;
        //cfg.setOptions(Config::OptionFsync | Config::OptionSemicolonSeparators | Config::OptionColonAssignmentForGroups | Config::OptionOpenBraceOnSeparateLine);
        try{
            cfg.readFile(inpath);
        }catch(const libconfig::FileIOException &fioex){
            std::cerr << "读取文件时发生 I/O 错误." << std::endl;
            return false;
        }catch(const libconfig::ParseException &pex){
            std::cerr << "文件解析错误： " << pex.getFile() << ":" << pex.getLine()
                      << " - " << pex.getError() << std::endl;
            return false;
        }
        try{
            cfg.writeFile(outpath);
            std::cerr << "格式化文件成功: " << outpath << std::endl;
        }catch(const libconfig::FileIOException &fileIOException){
            std::cerr << "文件写入过程 I/O 错误" << outpath << std::endl;
            return false;
        }
        return true;
    }
//private:
//    libconfig::Setting& settings; // 保存返回的setting
};


#endif //HVSONE_CONFIGURESETTINGS_H
