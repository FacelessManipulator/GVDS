//
// Created by yaowen on 5/3/19.
// 北航系统结构所-存储组
//


#define FUSE_USE_VERSION 31
#include <iostream>
#include <fuse3/fuse.h>
#include <fuse3/fuse_lowlevel.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <stdlib.h>
#include "fsutils.h"
using namespace hvs;

/*
 * return 0 on success, nonzero on failure.
 */

int main(/*int argc, char* argv[]*/) {
    //--help
//    int argc = 4;
//    char* argv[] = {const_cast<char *>("hvsfs"), const_cast<char *>("--rootdir=/home/yaowen/fuse/rootdir"),
//                    const_cast<char *>("/home/yaowen/fuse/mountpoint"), const_cast<char *>("--help")};
    // normal
    int argc = 3;
    char* argv[] = {const_cast<char *>("hvsfs"), const_cast<char *>("--rootdir=/home/yaowen/fuse/rootdir"),
                    const_cast<char *>("/home/yaowen/fuse/mountpoint")};
    //--version
//    int argc = 4;
//    char* argv[] = {const_cast<char *>("hvsfs"), const_cast<char *>("--rootdir=/home/yaowen/fuse/rootdir"),
//                    const_cast<char *>("/home/yaowen/fuse/mountpoint"), const_cast<char *>("--version")};
    //--unknown
//    int argc = 4;
//    char* argv[] = {const_cast<char *>("hvsfs"), const_cast<char *>("--rootdir=/home/yaowen/fuse/rootdir"),
//                    const_cast<char *>("/home/yaowen/fuse/mountpoint"), const_cast<char *>("--he")};

    //data
    struct hvsfs_state *hvs_data;
    int fuse_stat = 0;

    //options
    const struct fuse_opt option_spec[] = {
            OPTION("--rootdir=%s", rootdir),
            OPTION("-h", show_help),
            OPTION("--help", show_help),
            OPTION("-v", show_version),
            OPTION("--version", show_version),
            FUSE_OPT_END
    };
    struct options opt{};
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    if(fuse_opt_parse(&args, &opt, option_spec, hvsfs_opt_proc) == -1){
        return 1;
    }

    if ((argc < 3) || opt.show_help){
        //help info
        hvsfs_usage();
        fuse_opt_add_arg(&args, "--help");
        args.argv[0][0] = '\0';
    }

    if ((argc < 3) || opt.show_version){
        //version info
        fuse_opt_add_arg(&args, "--help");
        version_info();
        hvsfs_usage();
        args.argv[0][0] = '\0';
    }

    //privilege
    if(getuid()==0 || geteuid() == 0){
        std::cout << "Runnig as root, dangerous!" << std::endl;
        return 1;
    }

    //private data
    hvs_data =  new(hvs::hvsfs_state);
    hvs_data->rootdir = realpath(opt.rootdir, nullptr);
    hvs_data->logfile = log_open();

    //fuse_main function
    fuse_stat = fuse_main(args.argc, args.argv, &hvs::hvsfs_oper, hvs_data);
    /*
    * The following error codes may be returned from fuse_main():
    *   1: Invalid option arguments
    *   2: No mount point specified
    *   3: FUSE setup failed
    *   4: Mounting failed
    *   5: Failed to daemonize (detach from session)
    *   6: Failed to set up signal handlers
    *   7: An error occured during the life of the file system
    */
    fuse_opt_free_args(&args);
    return fuse_stat;
}