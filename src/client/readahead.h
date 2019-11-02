//
// Created by miller on 10/31/19.
//
#pragma once

#include <atomic>
#include "client.h"
#include "hvs_struct.h"
#include "common/Thread.h"

#define CACHE_UNIT_SIZE 512

namespace hvs {
    class ClientReadAhead : public ClientModule {
    private:
        virtual void start() override;
        virtual void stop() override;
        void fetch_sector_remote(int buf_idx);

    public:
        enum Status {
            MAY_SEQ_READ,
            UNLIKE_SEQ_READ,
            ON_LINK,
            IN_BUFFER,
            NOT_FOUND,
        };

        struct BufArea {
            uint64_t sec_bg;
            std::map<uint64_t, uint64_t> sec_bufferd;
            unsigned int max_sec;
            char* buf;
            std::atomic_uint64_t epoch;
            std::map<uint64_t, std::future<RPCLIB_MSGPACK::object_handle>> ft_res_tab;
            std::map<uint64_t, std::shared_future<bool>> ft_waiting_tab;
            std::map<uint64_t, std::shared_ptr<std::promise<bool>>> prom_waiting_tab;
            std::pair<std::shared_ptr<IOProxyNode>, std::string> file;
            std::atomic_uint64_t onlink;
            std::atomic_uint64_t cur_sec_nr;
            int cursor;
            std::mutex mut;
            Status lookup(const std::string& filename, uint64_t sec_nr) {
                if (filename != file.second) return NOT_FOUND;
                if(sec_nr < sec_bg || sec_nr >= sec_bg + max_sec)
                    return NOT_FOUND;
                else if(sec_bufferd.count(sec_nr)) {
                    return IN_BUFFER;
                } else {
                    return ON_LINK;
                }
            }
            char* sector_addr(uint64_t sec_nr) {
                return buf + (sec_nr-sec_bg)*(1 << 18);
            }
        };

    private:
        std::string last_req_filename;
        uint64_t last_req_sec;
        uint64_t last_req_hit_ct;
        unsigned int hit_threshold;
        unsigned int max_onlink;
        BufArea bufs[2];
        std::mutex mut;

    public:
        ClientReadAhead(const char* name, Client* cli) : ClientModule(name, cli) {
            isThread = false;
            last_req_sec = 0;
            last_req_hit_ct = 0;
        }
        Status status(const std::string& rpath, uint64_t offset, uint64_t size, char* dest);
        Status fetch_cache(const std::string& filename, uint64_t offset, uint64_t size, char* dest);
        // release the sector buffer before offset
        bool wait_sector(int buf_idx, uint64_t sec_nr);
        bool set_task(std::shared_ptr<IOProxyNode> iop, const std::string& filename,
                      int buf_idx, uint64_t sec_bg, unsigned int sec_max, uint64_t epoch);
        void clear_buf(const std::string& filepath);
    };
}  // namespace hvs