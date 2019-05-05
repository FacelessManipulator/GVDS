//
// Created by yaowen on 4/29/19.
// 北航系统结构所-存储组
//

#include <iostream>
#include <error.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fuse3/fuse.h>
#include <dirent.h>
#include <getopt.h>
#include "context.h"
#include "msg/rpc.h"
#include <io_proxy/rpc_types.h>
#include <vector>

namespace hvs{

    struct hvsfs_state{
        FILE *logfile;
    };

    struct options {
        int show_help;
        int show_version;
    };

    void hvs_init(){
        hvs::init_context();
    }

    #define HVSDATA ((struct hvsfs_state*) fuse_get_context()->private_data)
    #define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }

    void version_info(){
        std::cout << "Fuse library version " << fuse_pkgversion() << std::endl;
        fuse_lowlevel_version();
        std::cout << std::endl;
        std::cout << "Written by YaoXu <yaoxu@buaa.edu.cn>." << std::endl;
        std::cout << std::endl;
    }

    void hvsfs_usage(){
        std::cout << "usage: hvsfs [mount options] mountpoint" << std::endl;
    }

    int hvsfs_opt_proc (void *data, const char *arg, int key,struct fuse_args *outargs){
        if (arg [0]=='-' && arg [1]=='-'){
            std::cout<< "hvsfs: unknown option: " << arg << std::endl;
            exit(EXIT_FAILURE);
        }
        return 1;
    }


    FILE *log_open(){
        FILE *log_file;
        log_file = fopen("/tmp/hvsfs_log.txt", "w");
        if (log_file == nullptr){
            perror("logfile");
            exit(EXIT_FAILURE);
        }
        setvbuf(log_file, nullptr, _IONBF, 0);
        return log_file;
    }

    void log_msg(const char *format, ...)
    {
        va_list ap;
        va_start(ap, format);

        vfprintf(HVSDATA->logfile, format, ap);
        vfprintf(HVSDATA->logfile, "\n",nullptr);
    }

    void *hvsfs_init(struct fuse_conn_info *conn, struct fuse_config *cfg)
    {
        log_msg("init!");
        return HVSDATA;
    }

    //stat
    int hvsfs_getattr(const char *path, struct stat *stbuf,
                      struct fuse_file_info *fi)
    {
        log_msg("getattr!");
        int retstat = 0;
        memset(stbuf, 0, sizeof(struct stat));
        ConfigureSettings* config = hvs::HvsContext::get_context()->_config;
        auto ip = config->get<std::string>("ip");
        auto port = config->get<int>("rpc.port");
        RpcClient client(*ip, static_cast<const unsigned int>(*port));
        auto res = client.call("ioproxy_stat", path);
        auto retbuf = res->as<ioproxy_rpc_statbuffer>();
        stbuf->st_dev = static_cast<__dev_t>(retbuf.st_dev);
        stbuf->st_ino = static_cast<__ino_t>(retbuf.st_ino);
        stbuf->st_mode = static_cast<__mode_t>(retbuf.st_mode);
        stbuf->st_nlink = static_cast<__nlink_t>(retbuf.st_nlink);
        stbuf->st_uid = static_cast<__uid_t>(retbuf.st_uid);
        stbuf->st_gid = static_cast<__gid_t>(retbuf.st_gid);
        stbuf->st_rdev = static_cast<__dev_t>(retbuf.st_rdev);
        stbuf->st_size = retbuf.st_size;
        stbuf->st_atim.tv_nsec = retbuf.st_atim_tv_nsec;
        stbuf->st_atim.tv_sec = retbuf.st_atim_tv_sec;
        stbuf->st_mtim.tv_nsec = retbuf.st_mtim_tv_nsec;
        stbuf->st_mtim.tv_sec = retbuf.st_mtim_tv_sec;
        stbuf->st_ctim.tv_nsec = retbuf.st_ctim_tv_nsec;
        stbuf->st_ctim.tv_sec = retbuf.st_ctim_tv_sec;
        return retbuf.error_code;
    }

    int hvsfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                      off_t offset, struct fuse_file_info *fi,
                      enum fuse_readdir_flags flags)
    {
        int retstat = 0;
        if (strcmp(path, "/") != 0)
            return -ENOENT;
        log_msg("readdir!");
        ConfigureSettings* config = hvs::HvsContext::get_context()->_config;
        auto ip = config->get<std::string>("ip");
        auto port = config->get<int>("rpc.port");
        RpcClient client(*ip, static_cast<const unsigned int>(*port));
        auto res = client.call("ioproxy_readdir", path);
        for (const ioproxy_rpc_dirent &ent : res->as<std::vector<ioproxy_rpc_dirent>>()) {
            std::cout << ent.d_name << std::endl;
            if (filler(buf, ent.d_name.c_str(), nullptr, 0, static_cast<fuse_fill_dir_flags>(0)) != 0) {
                return -ENOMEM;
            }
            retstat = ent.error_code;
        }

        return retstat;
    }

    int hvsfs_open(const char *path, struct fuse_file_info *fi)
    {
        int retstat = 0;
        int fd = 0;

        log_msg("open!");

        return retstat;
    }

    int hvsfs_read(const char *path, char *buf, size_t size, off_t offset,
                   struct fuse_file_info *fi)
    {
        int retstat = 0;
        ConfigureSettings* config = hvs::HvsContext::get_context()->_config;
        auto ip = config->get<std::string>("ip");
        auto port = config->get<int>("rpc.port");
        RpcClient client(*ip, static_cast<const unsigned int>(*port));
        auto res = client.call("ioproxy_read", path, size, offset);
        ioproxy_rpc_readbuffer retbuf;
        retbuf = res->as<ioproxy_rpc_readbuffer>();
        memcpy(buf, retbuf.buf.c_str(), retbuf.buf.size());
        retstat = retbuf.size;
        return retstat;
    }

    int hvsfs_write(const char *path, const char *buf, size_t size, off_t offset,
                  struct fuse_file_info * fi)
    {
        int retstat = 0;
        ConfigureSettings* config = hvs::HvsContext::get_context()->_config;
        auto ip = config->get<std::string>("ip");
        auto port = config->get<int>("rpc.port");
        RpcClient client(*ip, static_cast<const unsigned int>(*port));
        auto res = client.call("ioproxy_write", path, buf, size, offset);
        retstat = res->as<int>();
        return retstat;
    }

    int hvsfs_access(const char *path, int mask)
    {
        int retstat = 0;

        return retstat;
    }

    int hvsfs_opendir(const char *path, struct fuse_file_info *fi)
    {
        int retstat = 0;

        return retstat;
    }

    void hvsfs_destroy(void *private_data)
    {
        log_msg("Destroy!");
//        log_msg(((struct hvsfs_state*)private_data)->logfile);
    }

    struct fuse_operations hvsfs_oper = {
            .getattr = hvsfs_getattr,
            .open = hvsfs_open,
            .read = hvsfs_read,
            .write = hvsfs_write,
            .opendir = hvsfs_opendir,
            .readdir = hvsfs_readdir,
            .init = hvsfs_init,
            .destroy = hvsfs_destroy,
            .access = hvsfs_access,

//            .releasedir = hvsfs_releasedir,
//            .readlink   = hvsfs_readlink,
//            .mknod      = hvsfs_mknod,
//            .mkdir      = hvsfs_mkdir,
//            .symlink    = hvsfs_symlink,
//            .unlink     = hvsfs_unlink,
//            .rmdir      = hvsfs_rmdir,
//            .rename     = hvsfs_rename,
//            .link       = hvsfs_link,
//            .chmod      = hvsfs_chmod,
//            .chown      = hvsfs_chown,
//            .truncate   = hvsfs_truncate,
//            .utimens    = hvsfs_utimens,

//            .flush      = hvsfs_flush,
//            .fsync      = hvsfs_fsync,
//            .release    = hvsfs_release,

//            .statfs     = hvsfs_statfs,
//            .create     = hvsfs_create,
    };
}
