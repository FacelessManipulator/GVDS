/**
 * Created by Yaowen Xu on 2019-04-01.
 * 作者: Yaowen Xu
 * 时间: 2019-04-01
 * 工程: HVSONE
 * 作者单位: 北京航空航天大学计算机学院-系统结构研究所
 */

#pragma once

#include <stdint.h>
#include <iostream>
#include <libconfig.h++>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "hvsdef.h"

//包含 libconfig 头文件
namespace hvs {
class ConfigureSettings {
 public:
  ConfigureSettings(const std::string& config_path);

  template <class T>
  std::optional<T> get(std::string path);
  template <class V>
  std::shared_ptr<std::vector<V>> get_list(std::string path);
  template <class T>
  bool add(const std::string& path, const T& value);
  bool writeFile(const char* outfile);
  bool vaild() { return _vaild; };

 protected:
  bool _format(const char* inpath, const char* outpath);
  template <class T>
  bool _add(libconfig::Setting& setting, std::string& path, const T& value);
  template <class V>
  bool _add(libconfig::Setting& setting, std::string& path,
            const std::vector<V>& value);
  libconfig::Setting& _build_group(libconfig::Setting& setting,
                                   std::string& path);
  libconfig::Setting* _push_forward(libconfig::Setting& setting,
                                    std::string& path);

 private:
  libconfig::Config baseCfg;  // 整体配置文件
  bool _vaild;
};

// Using these ugly stuff to convert type to libconfig typeid
template <typename T>
struct ConfigTypeConvert;

#define CONFIG_DECLAR_TYPE(X, Y)                       \
  template <>                                   \
  struct ConfigTypeConvert<X> {                 \
    static const libconfig::Setting::Type code; \
  };                                            

#define CONFIG_DEFINE_TYPE(X, Y)    \
const libconfig::Setting::Type ConfigTypeConvert<X>::code = Y;

CONFIG_DECLAR_TYPE(int, libconfig::Setting::TypeInt);
CONFIG_DECLAR_TYPE(int64_t, libconfig::Setting::TypeInt64);
CONFIG_DECLAR_TYPE(float, libconfig::Setting::TypeFloat);
CONFIG_DECLAR_TYPE(double, libconfig::Setting::TypeFloat);
CONFIG_DECLAR_TYPE(bool, libconfig::Setting::TypeBoolean);
CONFIG_DECLAR_TYPE(std::string, libconfig::Setting::TypeString);
CONFIG_DECLAR_TYPE(char*, libconfig::Setting::TypeString);

template <class T>
std::optional<T> ConfigureSettings::get(std::string path) {
  if (!vaild()) return {};
  T value;
  libconfig::Setting* setting_cur = _push_forward(baseCfg.getRoot(), path);
  if (setting_cur && setting_cur->lookupValue(path, value)) {
    return value;
  } else {
    return {};
  }
}

template <class V>
std::shared_ptr<std::vector<V>> ConfigureSettings::get_list(std::string path) {
  if (!vaild()) return nullptr;
  auto values = std::make_shared<std::vector<V>>();
  libconfig::Setting* setting_cur = _push_forward(baseCfg.getRoot(), path);
  if (setting_cur && setting_cur->exists(path) &&
      setting_cur->operator[](path.c_str()).isList()) {
    setting_cur = &(setting_cur->operator[](path.c_str()));
    for (int i = 0; i < setting_cur->getLength(); i++) {
      auto* value = &((*setting_cur)[i]);
      if (value->getType() == ConfigTypeConvert<V>::code) {
        V _buf = *value;
        values->push_back(_buf);
      }
    }
    return values;
  } else {
    return nullptr;
  }
}

template <class T>
bool ConfigureSettings::add(const std::string& path, const T& value) {
  if (!vaild()) return false;
  std::string m_path(path);
  libconfig::Setting& setting_cur = _build_group(baseCfg.getRoot(), m_path);
  return _add(setting_cur, m_path, value);
}

template <class T>
bool ConfigureSettings::_add(libconfig::Setting& setting, std::string& path,
                             const T& value) {
  libconfig::Setting& setting_cur = _build_group(setting, path);
  if (setting_cur.exists(path)) {
    return false;
  } else {
    setting_cur.add(path, ConfigTypeConvert<T>::code) = value;
    return true;
  }
}

template <class V>
bool ConfigureSettings::_add(libconfig::Setting& setting, std::string& path,
                             const std::vector<V>& value) {
  libconfig::Setting& setting_cur = _build_group(setting, path);
  if (setting_cur.exists(path)) {
    return false;
  } else {
    setting_cur.add(path, libconfig::Setting::TypeList);
    for (V iter : value) {
      setting_cur[path.c_str()].add(ConfigTypeConvert<V>::code) = iter;
    }
    return true;
  }
}
ConfigureSettings* init_config();
}  // namespace hvs