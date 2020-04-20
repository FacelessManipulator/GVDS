/*
 * @Author: Hanjie,Zhou 
 * @Date: 2020-03-18 15:51:15 
 * @Last Modified by:   Hanjie,Zhou 
 * @Last Modified time: 2020-03-18 15:51:15 
 */

#pragma once
#include "common/LRU.h"
#include <mutex>
#include <string>
#include <map>
#include "roaring/roaring.h"
#include "gvds_context.h"
#define _GVDS_SPACE_REPLICA_FILE_BITMAP_WITH_CID "user.gvds.r.fb%d"
#define _GVDS_SPACE_REPLICA_CENTER_IDS "user.gvds.r.c"
#define _GVDS_REPLICATE_SYNC_BLK 0x20000
// default blk size is 128KB
// default page size is 4kB
#define _GVDS_BASE_FS_PAGE_SIZE (1<<12)
#define _GVDS_BASE_FS_PAGE_MASK 0xFFF

namespace gvds {
    class IOProxy;
    class HvsFileHandler {
    public:
        HvsFileHandler(int fd_):fd(fd_), cids() {}
        ~HvsFileHandler() {
            if (fd != -1) {
                fd = -1;
                for (auto& [cid, bm]: bitmaps) {
                    roaring_bitmap_free(bm);
                }
                bitmaps.clear();
            }
        }

    public:
        // bitmap for replica file, map from cid to bitmap of each center
        int fd;
        std::map<int, roaring_bitmap_t*> bitmaps;
        std::string cids;
    private:
        std::mutex fh_mu;
    public:
        roaring_bitmap_t* get_bitmap(int cid, bool create = false);
        std::string get_rep_centers();
        void set_bitmap(int cid, roaring_bitmap_t* bitmap);
        bool flush_bitmap(int cid);
        bool flush_bitmap();
        bool punch_hole_local(uint64_t offset, size_t len);
        bool bitmap_remove_range_all();
        std::vector<std::pair<uint64_t, uint64_t>> find_holes_in_range(uint64_t from, uint64_t to);
        std::vector<std::pair<uint64_t, uint64_t>> find_data_in_range(uint64_t from, uint64_t to);
    };

    class FdManager {
    private:
        LRU<std::string, int> fds;
        std::map<int, std::shared_ptr<HvsFileHandler>> file_handlers;
        size_t max_fd_num;
        std::mutex fdm_mu;
    public:
        FdManager(IOProxy* ioProxy) : max_fd_num(500), iop(ioProxy) {}
        // should be local path
        int open(const std::string& path, int flags, int mode);
        int close(const std::string& path);
        int create(const std::string& path, int mode);
        int remove(const std::string& path);
        int expire(const std::string& path);
        int flush(const std::string& path);
        bool setMaxFdNum(size_t num) {
            // max_fd_num should bigger than 1
            max_fd_num = num > 1 ? num : 1;
        }
        std::shared_ptr<HvsFileHandler> get_file_handler(const std::string& path);
    private:
        void checkAndExpire();
        IOProxy* iop;
    };
};