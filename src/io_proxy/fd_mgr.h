//
// Created by miller on 6/18/19.
//

#pragma once
#include "common/LRU.h"
#include <string>

namespace hvs {
    class IOProxy;
    class FdManager {
    private:
        LRU<std::string, int> fds;
        size_t max_fd_num;
    public:
        FdManager(IOProxy* ioProxy) : max_fd_num(500), iop(ioProxy) {}
        // should be local path
        int open(const std::string& path, int flags, int mode);
        int close(const std::string& path);
        int flush(const std::string& path);
        bool setMaxFdNum(size_t num) {
            // max_fd_num should bigger than 1
            max_fd_num = num > 1 ? num : 1;
        }
    private:
        void checkAndExpire();
        IOProxy* iop;
    };
};