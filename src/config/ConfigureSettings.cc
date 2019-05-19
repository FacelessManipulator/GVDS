/**
 * Created by Yaowen Xu on 2019-02-26.
 * 作者: Yaowen Xu
 * 时间: 2019-02-26
 * 工程: Configlib
 * 作者单位: 北京航空航天大学计算机学院-系统结构研究所
 */

#include "config/ConfigureSettings.h"
using namespace libconfig;
using namespace hvs;
using namespace std;

CONFIG_DEFINE_TYPE(int, libconfig::Setting::TypeInt);
CONFIG_DEFINE_TYPE(int64_t, libconfig::Setting::TypeInt64);
CONFIG_DEFINE_TYPE(float, libconfig::Setting::TypeFloat);
CONFIG_DEFINE_TYPE(double, libconfig::Setting::TypeFloat);
CONFIG_DEFINE_TYPE(bool, libconfig::Setting::TypeBoolean);
CONFIG_DEFINE_TYPE(std::string, libconfig::Setting::TypeString);
CONFIG_DEFINE_TYPE(char*, libconfig::Setting::TypeString);

// 创建配置文件解析模块
ConfigureSettings::ConfigureSettings(const std::string &configfile)
    : _vaild(false) {
  try {
    baseCfg.readFile(configfile.c_str());
    _vaild = true;
  } catch (const FileIOException &fileIOException) {
    std::cerr << "无法读取配置文件: " << fileIOException.what() << std::endl;
  } catch (const ParseException &parseException) {
    std::cerr << "配置文件解析错误： " << parseException.getFile() << ":"
              << parseException.getLine() << " - " << parseException.getError()
              << std::endl;
  } catch (const SettingNameException &settingNameException) {
    std::cerr << "配置文件读取错误"<< settingNameException.what()
              << std::endl;
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

bool ConfigureSettings::writeFile(const char *outfile) {
  try {
    baseCfg.writeFile(outfile);
    // std::cerr << "配置文件更新成功: " << outfile << std::endl;
    return true;
  } catch (const FileIOException &fileIOException) {
    std::cerr << "文件写入过程 I/O 错误" << outfile << fileIOException.what()
              << std::endl;
    return false;
  } catch (const SettingException &settingException) {
    std::cerr << "文件写入错误" << outfile << settingException.what()
              << std::endl;
  }
}

hvs::ConfigureSettings* hvs::init_config(std::string configPath) {
  hvs::ConfigureSettings* _config = new hvs::ConfigureSettings(configPath);

  // 添加默认配置项
  _config->add("user.age", 1);
  _config->add("user.hello.world", false);
  // 目前由于template的限制，只能使用string包装，不能直接用字符串
  _config->add("user.world", string("any string"));
  _config->add("float", 3.14);
  _config->add("list", vector{1, 2, 3});

  _config->add("ip", string("0.0.0.0"));
  _config->add("log.path", string("/var/log/hvs.log"));
  _config->add("log.level", 10);
  _config->add("rpc.port", 9092);
  _config->add("rpc.workers", 100);
  _config->add("rpc.timeout", 3000);
  _config->add("rpc.retry", 3);
  _config->add("couchbase.address", string("192.168.10.235"));
  _config->add("couchbase.user", string("dev"));
  _config->add("couchbase.password", string("buaaica"));
  _config->add("rest.port", 9090);
  _config->add("rest.thread_num", 1);
  _config->add("couchbase.bucket", string("test"));
  // fuse config options
  _config->add("fuse.mountpoint", string("/mnt/hvs"));
  _config->add("fuse.foreground", true);
  _config->add("fuse.debug", true);
  _config->add("fuse.multithread", true);
  _config->add("fuse.auto_unmount", true);

  _config->add("ioproxy.scher", 6);
  _config->add("ioproxy.data_port", 9095);
  _config->add("ioproxy.data_buffer", 10240000);

  return _config;
}