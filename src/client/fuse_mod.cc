/*
 * @Author: Hanjie,Zhou
 * @Date: 2020-02-20 00:36:25
 * @Last Modified by:   Hanjie,Zhou
 * @Last Modified time: 2020-02-20 00:36:25
 */
#include "client/fuse_mod.h"
#include <cstdlib>

using namespace hvs;
using namespace std;

void ClientFuse::start() {
  // read config and compose the argv
  auto _config = HvsContext::get_context()->_config;
  auto mp = _config->get<string>("client.mountpoint");
  auto foreground = _config->get<bool>("client.foreground");
  auto debug = _config->get<bool>("client.debug");
  auto multithread = _config->get<bool>("client.multithread");
  auto auto_unmount = _config->get<bool>("client.auto_unmount");
  use_udt = _config->get<bool>("client.use_udt").value_or(false);
  async_mode = _config->get<bool>("client.async").value_or(false);
  readahead = _config->get<int>("client.readahead").value_or(0);
  snprintf(workers_argv, 32, "max_idle_threads=%u",
           _config->get<unsigned int>("client.fuse_workers").value_or(10));
  memcpy(mountpoint, mp.value_or("/mnt/hvs").c_str(),
         mp.value_or("/mnt/hvs").size() + 1);
  char *options[] = {
      const_cast<char *>("hvs_client"),
      const_cast<char *>("-f"),
      const_cast<char *>("-d"),
      const_cast<char *>("-s"),
      const_cast<char *>("-o"),
      const_cast<char *>("auto_unmount,max_read=524288,allow_other"),
  };
  fuse_argv[fuse_argc] = options[0];
  fuse_argc++;
  if (foreground.value_or(false)) {
    fuse_argv[fuse_argc] = options[1];
    fuse_argc++;
  }
  if (debug.value_or(false)) {
    fuse_argv[fuse_argc] = options[2];
    fuse_argc++;
  }
  if (!multithread.value_or(true)) {
    fuse_argv[fuse_argc] = options[3];
    fuse_argc++;
  }
  if (auto_unmount.value_or(true)) {
    fuse_argv[fuse_argc] = options[4];
    fuse_argv[fuse_argc + 1] = options[5];
    fuse_argc += 2;
  }
  // fuse worker threads
  // fuse_argv[fuse_argc] = options[4];
  // fuse_argv[fuse_argc+1] = workers_argv;
  // fuse_argc += 2;
  fuse_argv[fuse_argc] = mountpoint;
  fuse_argc++;
  fs_priv.client = client;
  fs_priv.fuse_client = this;
}

void ClientFuse::stop() {
  // exit(0);
}

void ClientFuse::run() {
  int fuse_stat = 0;
  fuse_stat = fuse_main(fuse_argc, fuse_argv, &hvsfs_oper, &fs_priv);
}