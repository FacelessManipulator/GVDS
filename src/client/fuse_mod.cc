#include "client/fuse_mod.h"
#include <cstdlib>

using namespace hvs;
using namespace std;

void ClientFuse::start() {
  // read config and compose the argv
  auto _config = HvsContext::get_context()->_config;
  auto mp = _config->get<string>("fuse.mountpoint");
  auto foreground = _config->get<bool>("fuse.foreground");
  auto debug = _config->get<bool>("fuse.debug");
  auto multithread = _config->get<bool>("fuse.multithread");
  auto auto_unmount = _config->get<bool>("fuse.auto_unmount");
  use_udt = _config->get<bool>("fuse.use_udt").value_or(true);
  memcpy(mountpoint, mp.value_or("/mnt/hvs").c_str(), mp.value_or("/mnt/hvs").size());
  char *options[] = {
      const_cast<char *>("hvs_client"), const_cast<char *>("-f"),
      const_cast<char *>("-d"),         const_cast<char *>("-s"),
      const_cast<char *>("-o"),         const_cast<char *>("auto_unmount"),
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
  if (multithread.value_or(true)) {
    fuse_argv[fuse_argc] = options[3];
    fuse_argc++;
  }
  if (auto_unmount.value_or(true)) {
    fuse_argv[fuse_argc] = options[4];
    fuse_argv[fuse_argc + 1] = options[5];
    fuse_argc += 2;
  }
  fuse_argv[fuse_argc] = mountpoint;
  fuse_argc ++;
  fs_priv.client = client;
  fs_priv.fuse_client = this;
}

void ClientFuse::stop() {
  //exit(0);
}

void ClientFuse::run() {
  int fuse_stat = 0;
  fuse_stat = fuse_main(fuse_argc, fuse_argv, &hvsfs_oper, &fs_priv);
}