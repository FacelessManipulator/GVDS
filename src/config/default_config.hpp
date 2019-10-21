#pragma once
#include <string>
#include <vector>
#include "config/ConfigureSettings.h"
#include "common/centerinfo.h"

namespace hvs {
using std::string;
using std::vector;
inline void default_config(hvs::ConfigureSettings* _config) {
  // 添加默认配置项
  _config->add("user.age", 1);
  _config->add("user.hello.world", false);
  // 目前由于template的限制，只能使用string包装，不能直接用字符串
  _config->add("user.world", string("any string"));
  _config->add("float", 3.14);
  _config->add("list", vector{1, 2, 3});

  _config->add("ip", string("127.0.0.1"));
  _config->add("manager_addr", string("127.0.0.1:9090"));
  _config->add("ioproxy.cid",string("1"));

  _config->add("log.path", string("/var/log/hvs.log"));
  _config->add("log.level", 10);
  _config->add("ioproxy.rpc_port", 9092);
  _config->add("ioproxy.rpc_workers", 6);
  _config->add("ioproxy.rpc_timeout", 3000);
  _config->add("ioproxy.rpc_retry", 3);
  _config->add("manager.couchbase_addr", string("127.0.0.1"));
  _config->add("manager.couchbase_user", string("dev"));
  _config->add("manager.couchbase_passwd", string("buaaica"));
  
  // fuse config options
  _config->add("client.mountpoint", string("/mnt/hvs"));
  _config->add("client.foreground", true);
  _config->add("client.debug", false);
  _config->add("client.multithread", true);
  _config->add("client.auto_unmount", true);
  _config->add("client.use_udt", false);

  // ioproxy config options
  _config->add("ioproxy.scher", 6);
  _config->add("ioproxy.data_port", 9095);
  _config->add("ioproxy.data_buffer", 10240000);
  _config->add("ioproxy.data_conn", 50);

  // client config options
  _config->add("client.data_port_begin", 9096);
  _config->add("client.data_port_end", 9150);
  _config->add("client.data_buffer", 10240000);

  // manager config options
  _config->add("manager.port", 9090);
  _config->add("manager.thread_num", 6);
  _config->add("manager.id", string("1"));

}
}  // namespace hvs