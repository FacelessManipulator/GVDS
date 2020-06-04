//
// Changed by mark on 3/18/20.
//
#pragma once

#include <atomic>
#include "client.h"
#include "gvds_struct.h"
#include "common/Thread.h"
#include "client/msg_mod.h"
#include "common/LRU.h"

#define CACHE_UNIT_SIZE 512
#define READAHEAD_DEBUG_LEVEL 20

namespace gvds {
class ClientReadAhead : public ClientModule {
    private:
        virtual void start() override;
        virtual void stop() override;

    public:
        enum Status {
            READAHEAD,
            ON_LINK,
            IN_BUFFER,
            NOT_FOUND,
        };

        struct BufArea {
            uint64_t sec_bg;
            std::map<uint64_t, uint64_t> sec_buffered;
            unsigned int max_sec;
            char* buf;
            std::atomic_uint64_t epoch;
            std::map<uint64_t, std::future<OpReply*>> ft_res_tab;
            std::map<uint64_t, std::shared_future<bool>> ft_waiting_tab;
            std::map<uint64_t, std::shared_ptr<std::promise<bool>>> prom_waiting_tab;
            std::pair<std::string, std::shared_ptr<IOProxyNode>> file;
            unsigned int sec_trigger;
            std::atomic_uint64_t onlink;
            std::atomic_uint64_t cur_sec_nr;
            std::mutex mut;
            BufArea(): cur_sec_nr(0), onlink(0), sec_bg(0), max_sec(0), buf(nullptr), epoch(0),  sec_trigger(0) {}
            Status lookup(uint64_t sec_nr) {
                if(sec_nr < sec_bg || sec_nr >= sec_bg + max_sec)
                    return NOT_FOUND;
                else if(sec_buffered.count(sec_nr)) {
                    return IN_BUFFER;
                } else {
                    return ON_LINK;
                }
            }
            char* sector_addr(uint64_t sec_nr) {
                return buf + (sec_nr-sec_bg)*(1 << 17);
            }
            bool hit_trigger(uint64_t sec_nr) {
                return (sec_nr == sec_trigger);
            }
        };

    private:
        typedef std::list<BufArea> BufList;
        typedef BufList::iterator BufIter;

        unsigned int readahead;
        unsigned int max_onlink;
        BufList bufs;
        LRU<std::string, BufIter> lruList;
        std::mutex mut;
        Status fetch_cache(BufIter bufIter, uint64_t offset, uint64_t size, char* dest, uint64_t& read_size);
        Status wait_sector(BufIter bufIter, uint64_t sec_nr);
        void append_prefetch(BufIter bufIter);
        void clear_buf(BufIter bufIter);
        void fetch_sector_remote(BufIter);

    public:
        ClientReadAhead(const char* name, Client* cli) : ClientModule(name, cli) {
            isThread = false;
        }
        Status status(const std::string& rpath, uint64_t offset, uint64_t size, char* dest, uint64_t& read_size);
        Status fetch_cache(const std::string& filename, uint64_t offset, uint64_t size, char* dest, uint64_t& read_size);
        // release the sector buffer before offset
        bool set_task(std::shared_ptr<IOProxyNode> iop, const std::string& filename,
                     uint64_t sec_bg, unsigned int sec_max, uint64_t epoch);
        void clear_buf(const std::string& filepath);
    };
}  // namespace gvds