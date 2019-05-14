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

    std::string ioproxy_ip;

    struct options {
        int show_help;
        int show_version;
        int hasip;
    };

    #define HVSDATA ((struct hvsfs_state*) fuse_get_context()->private_data)
    #define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }

    void hvs_init(){
        hvs::init_context();
        std::cout << "Remote IOProxy IP: " << ioproxy_ip.c_str() << std::endl <<"RPC Port:"
        <<*(hvs::HvsContext::get_context()->_config->get<int>("rpc.port"))<< std::endl;
    }

    void version_info(){
        std::cout << "Fuse library version " << fuse_pkgversion() << std::endl;
        fuse_lowlevel_version();
        std::cout << std::endl;
        std::cout << "Written by YaoXu <yaoxu@buaa.edu.cn>." << std::endl;
        std::cout << std::endl;
    }

    void hvsfs_usage(){
        std::cout << "usage: hvsfs [mount options] <mountpoint> [--ip <ip-address>]" << std::endl;
    }

    int hvsfs_opt_proc (void *data, const char *arg, int key,struct fuse_args *outargs){
        if (arg [0]=='-' && arg [1]=='-'){
            std::cout<< "hvsfs: unknown option: " << arg << std::endl;
            exit(EXIT_FAILURE);
        }
        std::string arg_str(arg);
        if (arg_str.find('.')!=std::string::npos){
            ioproxy_ip = arg;
            return 0;
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
        memset(stbuf, 0, sizeof(struct stat));
        ConfigureSettings* config = hvs::HvsContext::get_context()->_config;
        auto ip = new std::string(ioproxy_ip.c_str());
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

    // stat multi
    std::vector<hvs::ioproxy_rpc_buffer> hvsfs_getattrs(std::vector<std::string> paths)
    {
      
        auto ip = new std::string(ioproxy_ip.c_str());
        ConfigureSettings* config = hvs::HvsContext::get_context()->_config;
        auto port = config->get<int>("rpc.port");
        RpcClient client(*ip, static_cast<const unsigned int>(*port));
        auto res = client.call("ioproxy_stat_multi", paths)->as<std::vector<hvs::ioproxy_rpc_buffer>>();
        return res;
    }


    int hvsfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                      off_t offset, struct fuse_file_info *fi,
                      enum fuse_readdir_flags flags)
    {
        log_msg("readdir!");
        int retstat = 0;
        ConfigureSettings* config = hvs::HvsContext::get_context()->_config;
        auto ip = new std::string(ioproxy_ip.c_str());
        auto port = config->get<int>("rpc.port");
        RpcClient client(*ip, static_cast<const unsigned int>(*port));
        auto res = client.call("ioproxy_readdir", path);
        for (const ioproxy_rpc_dirent &ent : res->as<std::vector<ioproxy_rpc_dirent>>()) {
            //std::cout << ent.d_name << std::endl;
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
        log_msg("open!");
        return retstat;
    }

    int hvsfs_read(const char *path, char *buf, size_t size, off_t offset,
                   struct fuse_file_info *fi)
    {
        int retstat = 0;
        ConfigureSettings* config = hvs::HvsContext::get_context()->_config;
        auto ip = new std::string(ioproxy_ip.c_str());
        auto port = config->get<int>("rpc.port");
        RpcClient client(*ip, static_cast<const unsigned int>(*port));
        auto res = client.call("ioproxy_read", path, size, offset);
        ioproxy_rpc_buffer retbuf;
        retbuf = res->as<ioproxy_rpc_buffer>();
        memcpy(buf, retbuf.buf.ptr, retbuf.buf.size);
        retstat = retbuf.buf.size;
        return retstat;
    }

    int hvsfs_write(const char *path, const char *buf, size_t size, off_t offset,
                  struct fuse_file_info * fi)
    {
        int retstat = 0;
        ConfigureSettings* config = hvs::HvsContext::get_context()->_config;
        auto ip = new std::string(ioproxy_ip.c_str());
        auto port = config->get<int>("rpc.port");
        RpcClient client(*ip, static_cast<const unsigned int>(*port));
        ioproxy_rpc_buffer _buffer(buf,size);
        auto res = client.call("ioproxy_write", path, _buffer, size, offset);
        retstat = res->as<int>();
        return retstat;
    }

    int hvsfs_access(const char *path, int mode)
    {
        int retstat = 0;
        ConfigureSettings* config = hvs::HvsContext::get_context()->_config;
        auto ip = new std::string(ioproxy_ip.c_str());
        auto port = config->get<int>("rpc.port");
        RpcClient client(*ip, static_cast<const unsigned int>(*port));
        auto res = client.call("ioproxy_access", path, mode);
        retstat = res->as<int>();
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
    }

    int hvsfs_truncate(const char *path, off_t offset, struct fuse_file_info *fi){
        int retstat = 0;
        auto ip = new std::string(ioproxy_ip.c_str());
        auto port = hvs::HvsContext::get_context()->_config->get<int>("rpc.port");
        RpcClient client(*ip, static_cast<const unsigned int>(*port));
        auto res = client.call("ioproxy_truncate", path, offset);
        retstat = res->as<int>();
        return retstat;
    }

    int hvsfs_readlink(const char *path, char *link, size_t size){
        int retstat = 0;
        auto ip = new std::string(ioproxy_ip.c_str());
        auto port = hvs::HvsContext::get_context()->_config->get<int>("rpc.port");
        RpcClient client(*ip, static_cast<const unsigned int>(*port));
        auto res = client.call("ioproxy_readdir", path, size);
        auto retstr = res->as<std::string>();
        memcpy(link, retstr.c_str(), retstr.size());
        retstat = static_cast<int>(retstr.size());
        return retstat;
    }

    int hvsfs_mknod (const char *path, mode_t mode, dev_t dev){
        int retstat = 0;

        return retstat;
    }
    int hvsfs_mkdir (const char *path, mode_t mode){
        int retstat = 0;
        auto ip = new std::string(ioproxy_ip.c_str());
        auto port = hvs::HvsContext::get_context()->_config->get<int>("rpc.port");
        RpcClient client(*ip, static_cast<const unsigned int>(*port));
        auto res = client.call("ioproxy_mkdir", path, mode);
        retstat = res->as<int>();
        return retstat;
    }

    int hvsfs_unlink (const char *path){
        int retstat = 0;
        auto ip = new std::string(ioproxy_ip.c_str());
        auto port = hvs::HvsContext::get_context()->_config->get<int>("rpc.port");
        RpcClient client(*ip, static_cast<const unsigned int>(*port));
        auto res = client.call("ioproxy_unlink", path);
        retstat = res->as<int>();
        return retstat;
    }

    int hvsfs_rmdir (const char *path){
        int retstat = 0;
        auto ip = new std::string(ioproxy_ip.c_str());
        auto port = hvs::HvsContext::get_context()->_config->get<int>("rpc.port");
        RpcClient client(*ip, static_cast<const unsigned int>(*port));
        auto res = client.call("ioproxy_rmdir", path);
        retstat = res->as<int>();
        return retstat;
    }

    int hvsfs_symlink (const char *path, const char *newpath){
        int retstat = 0;
        std::cout << "TODO: 软连接操作有问题，待修复！" << std::endl;
//        std::string root("/");
//        std::string paths(path);
//        std::string fullpath = root+paths;
//        auto ip = new std::string(ioproxy_ip.c_str());
//        auto port = hvs::HvsContext::get_context()->_config->get<int>("rpc.port");
//        RpcClient client(*ip, static_cast<const unsigned int>(*port));
//        auto res = client.call("ioproxy_symlink", fullpath.c_str(), newpath);
//        retstat = res->as<int>();
        return retstat;
    }

    int hvsfs_rename (const char *path, const char *newpath, unsigned int flags){
        int retstat = 0;
        auto ip = new std::string(ioproxy_ip.c_str());
        auto port = hvs::HvsContext::get_context()->_config->get<int>("rpc.port");
        RpcClient client(*ip, static_cast<const unsigned int>(*port));
        auto res = client.call("ioproxy_rename", path, newpath);
        retstat = res->as<int>();
        return retstat;
    }

    int hvsfs_link (const char *path, const char *newpath){
        int retstat = 0;
        std::cout << "TODO: 硬连接操作有问题，待修复！" << std::endl;
//        auto ip = new std::string(ioproxy_ip.c_str());
//        auto port = hvs::HvsContext::get_context()->_config->get<int>("rpc.port");
//        RpcClient client(*ip, static_cast<const unsigned int>(*port));
//        auto res = client.call("ioproxy_link", path, newpath);
//        retstat = res->as<int>();
        return retstat;
    }

    int hvsfs_chmod (const char *path, mode_t mode, struct fuse_file_info *fi){
        int retstat = 0;
        auto ip = new std::string(ioproxy_ip.c_str());
        auto port = hvs::HvsContext::get_context()->_config->get<int>("rpc.port");
        RpcClient client(*ip, static_cast<const unsigned int>(*port));
        auto res = client.call("ioproxy_chmod", path, mode);
        retstat = res->as<int>();
        return retstat;
    }

    int hvsfs_chown (const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi){
        int retstat = 0;
        auto ip = new std::string(ioproxy_ip.c_str());
        auto port = hvs::HvsContext::get_context()->_config->get<int>("rpc.port");
        RpcClient client(*ip, static_cast<const unsigned int>(*port));
        auto res = client.call("ioproxy_chown", path, uid, gid);
        retstat = res->as<int>();
        return retstat;
    }

    int hvsfs_statfs (const char *, struct statvfs *){
        int retstat = 0;

        return retstat;
    }

    int hvsfs_flush (const char *, struct fuse_file_info *){
        int retstat = 0;

        return retstat;
    }

    int hvsfs_release (const char *, struct fuse_file_info *){
        int retstat = 0;

        return retstat;
    }

    int hvsfs_fsync (const char *, int, struct fuse_file_info *){
        int retstat = 0;

        return retstat;
    }

    int hvsfs_releasedir (const char *, struct fuse_file_info *){
        int retstat = 0;

        return retstat;
    }

    int hvsfs_create (const char *path, mode_t mode, struct fuse_file_info *){
        int retstat = 0;
        auto ip = new std::string(ioproxy_ip.c_str());
        auto port = hvs::HvsContext::get_context()->_config->get<int>("rpc.port");
        RpcClient client(*ip, static_cast<const unsigned int>(*port));
        auto res = client.call("ioproxy_create", path, mode);
        retstat = res->as<int>();
        return retstat;
    }

    int hvsfs_utimens (const char *path, const struct timespec tv[2],
                    struct fuse_file_info *fi){
        int retstat = 0;
        long int sec0n = tv[0].tv_nsec;
        long int sec0s = tv[0].tv_sec;
        long int sec1n = tv[1].tv_nsec;
        long int sec1s = tv[1].tv_sec;
        auto ip = new std::string(ioproxy_ip.c_str());
        auto port = hvs::HvsContext::get_context()->_config->get<int>("rpc.port");
        RpcClient client(*ip, static_cast<const unsigned int>(*port));
        auto res = client.call("ioproxy_utimes", path, sec0n, sec0s, sec1n, sec1s);
        retstat = res->as<int>();
        return retstat;
    }

    struct fuse_operations hvsfs_oper = {
            .getattr    = hvsfs_getattr,
            .readlink   = hvsfs_readlink,
            .mknod      = hvsfs_mknod, // TODO: create函数重复，暂时不做
            .mkdir      = hvsfs_mkdir,
            .unlink     = hvsfs_unlink,
            .rmdir      = hvsfs_rmdir,
            .symlink    = hvsfs_symlink, // TODO: 软连接操作暂时有问题，待修复；
            .rename     = hvsfs_rename,
            .link       = hvsfs_link,   // TODO: 硬连接操作暂时有问题，待修复；
            .chmod      = hvsfs_chmod, // TODO: 虚拟数据空间相关，涉及到权限，之后需要统一修改；
            .chown      = hvsfs_chown, // TODO: 虚拟数据空间相关，涉及到权限，之后需要统一修改；
            .truncate   = hvsfs_truncate,
            .open       = hvsfs_open,
            .read       = hvsfs_read,
            .write      = hvsfs_write,
            .statfs     = hvsfs_statfs, // TODO: vfs 相关，暂时不做
            .flush      = hvsfs_flush, // TODO: cache 相关，暂时不做
            .release    = hvsfs_release, // TODO: 未保存文件fd, 暂时不做
            .fsync      = hvsfs_fsync, // TODO: caced 相关暂时不做
            .setxattr   = NULL,
            .getxattr   = NULL,
            .listxattr  = NULL,
            .removexattr= NULL,
            .opendir    = hvsfs_opendir,
            .readdir    = hvsfs_readdir,
            .releasedir = hvsfs_releasedir, // TODO: 未保存DIR, 暂时不做
            .fsyncdir   = NULL,
            .init       = hvsfs_init,
            .destroy    = hvsfs_destroy,
            .access     = hvsfs_access,
            .create     = hvsfs_create,
            .lock       = NULL,
            .utimens    = hvsfs_utimens,
            .bmap       = NULL,
            .ioctl      = NULL,
            .poll       = NULL,
            .write_buf  = NULL,
            .read_buf   = NULL,
            .flock      = NULL,
            .fallocate  = NULL,
    };
}
