/**
 * Created by Yaowen Xu on 2019-02-26.
 * 作者: Yaowen Xu
 * 时间: 2019-02-26
 * 工程: Configlib
 * 作者单位: 北京航空航天大学计算机学院-系统结构研究所
 */

#include "config/ConfigureSettings.h"
#include <boost/filesystem.hpp>
#include "config/default_config.hpp"

using namespace libconfig;
using namespace gvds;
using namespace std;

CONFIG_DEFINE_TYPE(int, libconfig::Setting::TypeInt);
CONFIG_DEFINE_TYPE(int64_t, libconfig::Setting::TypeInt64);
CONFIG_DEFINE_TYPE(float, libconfig::Setting::TypeFloat);
CONFIG_DEFINE_TYPE(double, libconfig::Setting::TypeFloat);
CONFIG_DEFINE_TYPE(bool, libconfig::Setting::TypeBoolean);
CONFIG_DEFINE_TYPE(std::string, libconfig::Setting::TypeString);
CONFIG_DEFINE_TYPE(char*, libconfig::Setting::TypeString);

// 创建配置文件解析模块
ConfigureSettings::ConfigureSettings(const std::string& configfile)
    : _vaild(false) {
  try {
    baseCfg.readFile(configfile.c_str());
    _vaild = true;
  } catch (const FileIOException& fileIOException) {
    std::cerr << "无法读取配置文件: " << fileIOException.what() << std::endl;
  } catch (const ParseException& parseException) {
    std::cerr << "配置文件解析错误： " << parseException.getFile() << ":"
              << parseException.getLine() << " - " << parseException.getError()
              << std::endl;
  } catch (const SettingNameException& settingNameException) {
    std::cerr << "配置文件读取错误" << settingNameException.what() << std::endl;
  }
}

bool ConfigureSettings::_format(const char* inpath, const char* outpath) {
  libconfig::Config cfg;
  // cfg.setOptions(Config::OptionFsync | Config::OptionSemicolonSeparators |
  // Config::OptionColonAssignmentForGroups |
  // Config::OptionOpenBraceOnSeparateLine);
  try {
    cfg.readFile(inpath);
  } catch (const libconfig::FileIOException& fioex) {
    std::cerr << "读取文件时发生 I/O 错误." << std::endl;
    return false;
  } catch (const libconfig::ParseException& pex) {
    std::cerr << "文件解析错误： " << pex.getFile() << ":" << pex.getLine()
              << " - " << pex.getError() << std::endl;
    return false;
  }
  try {
    cfg.writeFile(outpath);
    std::cerr << "格式化文件成功: " << outpath << std::endl;
  } catch (const libconfig::FileIOException& fileIOException) {
    std::cerr << "文件写入过程 I/O 错误" << outpath << std::endl;
    return false;
  }
  return true;
}

libconfig::Setting* ConfigureSettings::_push_forward(
    libconfig::Setting& setting, std::string& path) {
  size_t pos = 0;
  std::string token;
  libconfig::Setting* setting_cur = &baseCfg.getRoot();
  while ((pos = path.find(".")) != std::string::npos) {
    token = path.substr(0, pos);
    if (!setting_cur->exists(token)) {
      return nullptr;
    } else if (!setting_cur->operator[](token.c_str()).isGroup()) {
      return nullptr;
    } else {
      setting_cur = &(setting_cur->operator[](token.c_str()));
      // group exists. pass
    }
    path.erase(0, pos + 1);
  }
  return setting_cur;
}

libconfig::Setting& ConfigureSettings::_build_group(libconfig::Setting& setting,
                                                    std::string& path) {
  size_t pos = 0;
  std::string token;
  libconfig::Setting* setting_cur = &setting;
  while ((pos = path.find(".")) != std::string::npos) {
    token = path.substr(0, pos);
    if (!setting_cur->exists(token)) {
      setting_cur->add(token, libconfig::Setting::TypeGroup);
    } else if (!setting_cur->operator[](token.c_str()).isGroup()) {
      ASSERT(false, "config value can't add sub value");
    } else {
      // group exists. pass
    }
    setting_cur = &(setting_cur->operator[](token.c_str()));
    path.erase(0, pos + 1);
  }
  return *setting_cur;
}

bool ConfigureSettings::writeFile(const char* outfile) {
  try {
    baseCfg.writeFile(outfile);
    // std::cerr << "配置文件更新成功: " << outfile << std::endl;
    return true;
  } catch (const FileIOException& fileIOException) {
    std::cerr << "文件写入过程 I/O 错误" << outfile << fileIOException.what()
              << std::endl;
    return false;
  } catch (const SettingException& settingException) {
    std::cerr << "文件写入错误" << outfile << settingException.what()
              << std::endl;
  }
}

gvds::ConfigureSettings* gvds::init_config(const std::string& config_path) {
  gvds::ConfigureSettings* _config = nullptr;
  std::list<std::string> default_config_paths{
      "./gvds.conf", "/etc/gvds/gvds.conf", "/opt/gvds/gvds.conf"};
  if (!config_path.empty()) {
    default_config_paths.push_front(config_path);
  }
  for (auto& path : default_config_paths) {
    if (boost::filesystem::exists(path)) {
      _config = new gvds::ConfigureSettings(path);
      if(!_config->vaild()) {
        return nullptr;
      }
      std::cout << "GVDS Manager using config file: [" << path << "]." << std::endl;
      // default config code was moved to src/config/default_config.hpp
      default_config(_config);
      return _config;
    }
  }
  std::cerr << "ERROR: can't find gvds.conf in [";
  for(auto& path : default_config_paths) {
    std::cerr << "\"" << path << "\", ";
  }
  std::cerr << "]" << std::endl;
  return nullptr;
}