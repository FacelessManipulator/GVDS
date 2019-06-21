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

  _config->add("ip", string("0.0.0.0"));
  _config->add("manager_addr", vector{string("127.0.0.1")});
  _config->add("log.path", string("/var/log/hvs.log"));
  _config->add("log.level", 10);
  _config->add("rpc.port", 9092);
  _config->add("rpc.workers", 6);
  _config->add("rpc.timeout", 3000);
  _config->add("rpc.retry", 3);
  _config->add("couchbase.address", string("192.168.10.235"));
  _config->add("couchbase.user", string("dev"));
  _config->add("couchbase.password", string("buaaica"));
  _config->add("rest.port", 9090);
  _config->add("rest.thread_num", 10);
  _config->add("couchbase.bucket", string("test"));
  
  // fuse config options
  _config->add("fuse.mountpoint", string("/mnt/hvs"));
  _config->add("fuse.foreground", true);
  _config->add("fuse.debug", false);
  _config->add("fuse.multithread", true);
  _config->add("fuse.auto_unmount", true);
  _config->add("fuse.use_udt", true);

  // ioproxy config options
  _config->add("ioproxy.scher", 6);
  _config->add("ioproxy.data_port", 9095);
  _config->add("ioproxy.data_buffer", 10240000);
  _config->add("ioproxy.data_conn", 50);

  // client config options
  _config->add("client.data_port_begin", 9096);
  _config->add("client.data_port_end", 9150);
  _config->add("client.data_buffer", 10240000);

  // storage 本地挂载的文件系统（lustre等）
  _config->add("storage", string("/tmp/hvs/tests/data/"));
  // manager config options
  _config->add("manager.port", 9090);
  _config->add("manager.thread_num", 6);



_config->add("center_information", string("center_id1,center_id2,centerid3"));
_config->add("center_id1",string("192,168,1,2|192,168,1,3"));


_config->add("default_ip",string("127.0.0.1"));
_config->add("default_port",string("9090"));


_config->add("center_information_old", string("{"\
  "\"center1\":{"\
    "\"centerID\":\"1\","\
    "\"centerName\":\"Beijing\","\
    "\"managerPort\":\"9090\","\
    "\"managerIP\":\"127.0.0.1\""\
  "},"\
  "\"center2\":{"\
    "\"centerID\":\"2\","\
    "\"centerName\":\"Beijing\","\
    "\"managerPort\":\"9090\","\
    "\"managerIP\":\"127.0.0.1\""\
  "},"\
    "\"center3\":{"\
    "\"centerID\":\"3\","\
    "\"centerName\":\"Beijing\","\
    "\"managerPort\":\"9090\","\
    "\"managerIP\":\"127.0.0.1\""\
  "},"\
    "\"center4\":{"\
    "\"centerID\":\"4\","\
    "\"centerName\":\"Beijing\","\
    "\"managerPort\":\"9090\","\
    "\"managerIP\":\"127.0.0.1\""\
  "},"\
    "\"center5\":{"\
    "\"centerID\":\"5\","\
    "\"centerName\":\"Beijing\","\
    "\"managerPort\":\"9090\","\
    "\"managerIP\":\"127.0.0.1\""\
  "}"\
"}"));


}
}  // namespace hvs