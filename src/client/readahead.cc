//
// Created by miller on 10/31/19.
//

#include "client/readahead.h"
#include "client/client.h"
#include "client/msg_mod.h"

using namespace hvs;
using namespace std;

void ClientReadAhead::start() {
    auto _config = HvsContext::get_context()->_config;
    hit_threshold = _config->get<int>("client.seq_threshold").value_or(1);
    max_onlink = _config->get<int>("client.onlink").value_or(64);
}

void ClientReadAhead::stop() {}

void ClientReadAhead::fetch_sector_remote() {
    std::shared_ptr<IOProxyNode> iop = current_speedup.first;
    std::string filename = current_speedup.second;

    auto rpcc = client->rpc->rpc_channel(iop, false, 100);
    // the callback function could be used to trigger some event
    while (cur_onlink < max_onlink && cur_sec_nr < bufs[0].sec_bg + bufs[0].max_sec) {
        cur_onlink++;
        uint64_t sec_nr = cur_sec_nr++;
        auto ft = rpcc->_client->async_call_callback("ioproxy_read",
                                                     [this, sec_nr, iop, filename]() {
                                                         cur_onlink--;
                                                         RPCLIB_MSGPACK::object_handle obj_hd;
                                                         bufs[0].mut.lock();
                                                         if(bufs[0].ft_res_tab.count(sec_nr) > 0) {
                                                             obj_hd = bufs[0].ft_res_tab[sec_nr].get();
                                                             bufs[0].ft_res_tab.erase(sec_nr);
                                                         }
                                                         bufs[0].mut.unlock();
                                                         if (obj_hd.zone() == nullptr)
                                                             return ;
                                                         auto retbuf = obj_hd.as<ioproxy_rpc_buffer>();
                                                         if (retbuf.error_code != 0) {
                                                             // stat failed on remote server
                                                             return;
                                                         }
                                                         memcpy(bufs[0].sector_addr(sec_nr), retbuf.buf.ptr,
                                                                retbuf.buf.size);
                                                         bufs[0].mut.lock();
                                                         bufs[0].sec_bufferd.insert(sec_nr);
                                                         // if there are someone who waiting for this sector, wake it up
                                                         if (bufs[0].prom_waiting_tab.count(sec_nr) > 0) {
                                                             bufs[0].prom_waiting_tab[sec_nr]->set_value(
                                                                     retbuf.error_code == 0);
                                                             bufs[0].prom_waiting_tab.erase(sec_nr);
                                                         }
                                                         bufs[0].mut.unlock();
                                                         dout(20) << "prefetch: " << filename << " sector: " << sec_nr
                                                                  << " size:" << retbuf.buf.size << dendl;
                                                         if (iop == current_speedup.first &&
                                                             filename == current_speedup.second &&
                                                             cur_onlink < max_onlink &&
                                                             cur_sec_nr + 1 < bufs[0].sec_bg + bufs[0].max_sec) {
                                                             fetch_sector_remote();
                                                         }
                                                     }, filename, 1 << 18, sec_nr << 18, 0);
        // TODO: we are not sure the code below 100% run before the callback function
        bufs[0].ft_res_tab[sec_nr] = std::move(ft);
    }
}

ClientReadAhead::Status ClientReadAhead::status(const std::string& rpath, uint64_t offset, uint64_t size, char*& dest) {
    uint64_t sector_bg = offset >> 18;
//            uint64_t sector_nr = (size >> 8) + (1-~(size&0xff)); // = upper(size/256)
    uint64_t sector_nr = 1; // currently fuse only send at most 128KB request
    mut.lock();
    if(rpath == current_speedup.second) {
        mut.unlock();
        auto st = fetch_cache(offset, size, dest);
        mut.lock();
        if (UNLIKE_SEQ_READ == st) {
            current_speedup.first.reset();
            current_speedup.second.clear();
        }
        mut.unlock();
        return st;
    }
    mut.unlock();
    std::lock_guard<std::mutex> lock(mut);
    if (last_req_filename != rpath) {
        last_req_filename = rpath;
        last_req_hit_ct = 1;
        return UNLIKE_SEQ_READ;
    }
    if (last_req_sec > sector_bg) {
        last_req_sec = sector_bg;
        return UNLIKE_SEQ_READ;
    }
    if (last_req_hit_ct < hit_threshold) {
        last_req_hit_ct ++;
        return UNLIKE_SEQ_READ;
    }
    return MAY_SEQ_READ;
}

ClientReadAhead::Status ClientReadAhead::fetch_cache(uint64_t offset, uint64_t size, char*& dest) {
    uint64_t sector_bg = offset >> 18;
    Status st = bufs[0].lookup(sector_bg);
    if(st == UNLIKE_SEQ_READ) {
        return UNLIKE_SEQ_READ;
    } else if (st == IN_BUFFER) {
        dest = bufs[0].sector_addr(sector_bg) + (offset&0x3ffff);
        if (dest + size > bufs[0].sector_addr(bufs[0].sec_bg + bufs[0].max_sec))
            return UNLIKE_SEQ_READ;
//                    dest = bufs[0].buf + (offset - (bufs[0].sec_bg << 18));
        return IN_BUFFER;
    } else if (st == ON_LINK) {
        // wait until fulfilled
        if (wait_sector(sector_bg)) {
            dest = bufs->sector_addr(sector_bg) + (offset&0x3ffff);
            if (dest + size > bufs[0].sector_addr(bufs[0].sec_bg + bufs[0].max_sec))
                return UNLIKE_SEQ_READ;
//                        dest = bufs[0].buf + (offset - (bufs[0].sec_bg << 18));
            return IN_BUFFER;
        } else {
            return UNLIKE_SEQ_READ;
        }
    } else {
        // impossible to reach here
        return UNLIKE_SEQ_READ;
    }
}

// release the sector buffer before offset
bool ClientReadAhead::wait_sector(uint64_t sec_nr) {
    bufs[0].mut.lock();
    if(bufs[0].ft_waiting_tab.count(sec_nr) == 0) {
        bufs[0].prom_waiting_tab[sec_nr] = std::make_shared<std::promise<bool>>();
        bufs[0].ft_waiting_tab[sec_nr] = std::shared_future(bufs[0].prom_waiting_tab[sec_nr]->get_future());
    }
    auto sft = bufs[0].ft_waiting_tab[sec_nr];
    bufs[0].mut.unlock();
    auto st = sft.wait_for(std::chrono::milliseconds(1000));
    bufs[0].mut.lock();
    bool ret = (bufs[0].ft_waiting_tab.count(sec_nr) == 0 || st==std::future_status::ready && sft.get());
    if(bufs[0].ft_waiting_tab.count(sec_nr) > 0) {
        bufs[0].prom_waiting_tab.erase(sec_nr);
        bufs[0].ft_waiting_tab.erase(sec_nr);
    }
    bufs[0].mut.unlock();
    return ret;
}

void ClientReadAhead::clear_buf() {
    if(current_speedup.second.empty())
        return;
    std::lock_guard<std::mutex> lock(mut);
    std::lock_guard<std::mutex> lock2(bufs[0].mut);
    for(auto& prom:bufs[0].prom_waiting_tab) {
        prom.second->set_value(false);
    }
    current_speedup.first.reset();
    current_speedup.second.clear();
    cur_sec_nr = 0;
    cur_onlink = 0;
    bufs[0].sec_bg = 0;
    bufs[0].max_sec = 0;
    bufs[0].sec_bufferd.clear();
    bufs[0].ft_res_tab.clear();
    bufs[0].prom_waiting_tab.clear();
    bufs[0].ft_waiting_tab.clear();
}

bool ClientReadAhead::set_task(std::shared_ptr<IOProxyNode> iop, const std::string& filename, uint64_t sec_bg, uint64_t sec_max=100) {
    // no need to speed up, already have this task
    clear_buf();
    {
        std::lock_guard<std::mutex> lock(mut);
        std::lock_guard<std::mutex> lock2(bufs[0].mut);
        if (bufs[0].ft_res_tab.size() != 0) {
            // cannot set new task if there are onlink or waiting thread
            return false;
        }
        if (filename == current_speedup.second && sec_bg < bufs[0].sec_bg + bufs[0].max_sec)
            return true;
        for(auto& prom:bufs[0].prom_waiting_tab) {
            prom.second->set_value(false);
        }
        current_speedup.first = iop;
        current_speedup.second = filename;
        cur_sec_nr = sec_bg;
        cur_onlink = 0;
        bufs[0].sec_bg = sec_bg;
        bufs[0].max_sec = sec_max;
        bufs[0].buf = (char *) realloc(bufs[0].buf, sec_max << 18);
        fetch_sector_remote();
        dout(15) << "readahead task: " << filename << " sector: " << sec_bg << dendl;
    }
    return true;
}