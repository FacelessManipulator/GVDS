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


namespace hvs{
    struct hvsfs_state{
        FILE *logfile;
        char *rootdir;
    };

    struct options {
        const char *rootdir;
        int show_help;
        int show_version;
    };

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
        std::cout << "usage: hvsfs [mount options] --rootdir=[dir] mountpoint" << std::endl;
    }

    int hvsfs_opt_proc (void *data, const char *arg, int key,struct fuse_args *outargs){
        if (arg [0]=='-'){
            std::cout<< "hvsfs: unknown option: " << arg << std::endl;
            exit(EXIT_FAILURE);
        }
        return 1;
    }


    FILE *log_open(){
        FILE *log_file;
        log_file = fopen("/home/yaowen/CLionProjects/FUSE/log.txt", "w");
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

    void hvsfs_fullpath(char fpath[PATH_MAX], const char *path){
        //
        
        strcpy(fpath, HVSDATA->rootdir);
        strncat(fpath, path, PATH_MAX);
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
        int retstat = 0;
        char fpath[PATH_MAX];
        hvsfs_fullpath(fpath, path);
        memset(stbuf, 0, sizeof(struct stat));
        log_msg(fpath);
        retstat = lstat(fpath, stbuf);
        if (retstat < 0){
            retstat = -errno;
        }
        log_msg("getattr!");
        return retstat;
    }

    int hvsfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                      off_t offset, struct fuse_file_info *fi,
                      enum fuse_readdir_flags flags)
    {
        int retstat = 0;
        if (strcmp(path, "/") != 0)
            return -ENOENT;
        DIR *dp;
        struct dirent *de;
        dp = (DIR *) (uintptr_t) fi->fh;
        de = readdir(dp);
        if (de == nullptr) {
            return -errno;
        }
        do {
            if (filler(buf, de->d_name, nullptr, 0, static_cast<fuse_fill_dir_flags>(0)) != 0) {
                return -ENOMEM;
            }
        } while ((de = readdir(dp)) != nullptr);
        log_msg("readdir!");
        return retstat;
    }

    int hvsfs_open(const char *path, struct fuse_file_info *fi)
    {
        int retstat = 0;
        int fd;
        char fpath[PATH_MAX];
        hvsfs_fullpath(fpath, path);
        fd = open(fpath, fi->flags);
        if (fd < 0){
            log_msg("open error!");
            retstat = -errno;
        }
        fi->fh = static_cast<uint64_t>(fd);
        log_msg("open!");

        return retstat;
    }

    int hvsfs_read(const char *path, char *buf, size_t size, off_t offset,
                   struct fuse_file_info *fi)
    {
        int retstat = 0;
        retstat = static_cast<int>(pread(static_cast<int>(fi->fh), buf, size, offset));
        log_msg("read!");
        return retstat;
    }

    int hvsfs_write(const char *path, const char *buf, size_t size, off_t offset,
                  struct fuse_file_info * fi)
    {
        int retstat = 0;
        retstat = static_cast<int>(pwrite(static_cast<int>(fi->fh), buf, size, offset));
        log_msg("wirte!");
        return retstat;
    }

    int hvsfs_access(const char *path, int mask)
    {
        int retstat = 0;
        char fpath[PATH_MAX];


        hvsfs_fullpath(fpath, path);

        retstat = access(fpath, mask);

        if (retstat < 0)
            retstat = -errno;

        return retstat;
    }

    int hvsfs_opendir(const char *path, struct fuse_file_info *fi)
    {
        DIR *dp;
        int retstat = 0;
        char fpath[PATH_MAX];
        hvsfs_fullpath(fpath, path);
        dp = opendir(fpath);
        if (dp == nullptr)
            retstat = -errno;

        fi->fh = static_cast<uint64_t>((intptr_t) dp);

        return retstat;
    }

    void hvsfs_destroy(void *private_data)
    {
        log_msg("Destroy!");
        log_msg(((struct hvsfs_state*)private_data)->rootdir);
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
