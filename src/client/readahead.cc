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

void ClientReadAhead::fetch_sector_remote(int buf_idx) {
    std::shared_ptr<IOProxyNode> iop = bufs[buf_idx].file.first;
    std::string filename = bufs[buf_idx].file.second;

    auto rpcc = client->rpc->rpc_channel(iop, false, 100);
    // the callback function could be used to trigger some event
    while (bufs[buf_idx].onlink < max_onlink && bufs[buf_idx].cur_sec_nr < bufs[buf_idx].sec_bg + bufs[buf_idx].max_sec) {
        bufs[buf_idx].onlink++;
        uint64_t sec_nr = bufs[buf_idx].cur_sec_nr++, cur_epoch = bufs[buf_idx].epoch;
        auto ft = rpcc->_client->async_call_callback("ioproxy_read",
                                                     [this, sec_nr, iop, filename, buf_idx, cur_epoch]() {
                                                         bufs[buf_idx].mut.lock();
                                                         if (bufs[buf_idx].epoch != cur_epoch) {
                                                             // out-of-date response, not expected
                                                             bufs[buf_idx].mut.unlock();
                                                             return;
                                                         } else {
                                                             bufs[buf_idx].onlink--;
                                                             // fetch the response data from rpclib's future
                                                             std::future<RPCLIB_MSGPACK::object_handle> ft_cur;
                                                             if (bufs[buf_idx].ft_res_tab.count(sec_nr) > 0) {
                                                                 ft_cur = std::move(bufs[buf_idx].ft_res_tab[sec_nr]);
                                                                 bufs[buf_idx].ft_res_tab.erase(sec_nr);
                                                             } else {
                                                                 // if future not exsits, means new task has been set
                                                                 bufs[buf_idx].mut.unlock();
                                                                 return;
                                                             }
                                                             bufs[buf_idx].mut.unlock();

                                                             // this callback function may be miss called even if there is a timeout
                                                             auto st = ft_cur.wait_for(chrono::milliseconds(100));
                                                             if (st != future_status::ready) return;
                                                             auto obj_hd = ft_cur.get();
                                                             auto retbuf = obj_hd.as<ioproxy_rpc_buffer>();
                                                             if (retbuf.error_code != 0) {
                                                                 // stat failed on remote server
                                                                 return;
                                                             }

                                                             // copy the data from response to buffer
                                                             memcpy(bufs[buf_idx].sector_addr(sec_nr), retbuf.buf.ptr,
                                                                    retbuf.buf.size);
                                                             bufs[buf_idx].mut.lock();
                                                             bufs[buf_idx].sec_bufferd[sec_nr] = retbuf.buf.size;
                                                             // if there are someone who waiting for this sector, wake it up
                                                             if (bufs[buf_idx].prom_waiting_tab.count(sec_nr) > 0) {
                                                                 bufs[buf_idx].prom_waiting_tab[sec_nr]->set_value(
                                                                         retbuf.error_code == 0);
                                                                 bufs[buf_idx].prom_waiting_tab.erase(sec_nr);
                                                             }
                                                             bufs[buf_idx].mut.unlock();
                                                             dout(20) << "prefetch: " << filename << " sector: "
                                                                      << sec_nr
                                                                      << " size:" << retbuf.buf.size << dendl;

                                                             // trigger the next call, it could trigger at least 1 remote fetch
                                                             if (retbuf.buf.size != 0 && bufs[buf_idx].onlink < max_onlink) {
                                                                 if (bufs[buf_idx].cur_sec_nr >= bufs[buf_idx].sec_bg + bufs[buf_idx].max_sec) {
                                                                     // if cur buf_idx is full, use next buf
                                                                     if (cur_epoch >= bufs[(buf_idx+1)%2].epoch) {
                                                                         // if cur_epoch > next buf's epoch, means cur_task newer/same with the next buf's task
                                                                         set_task(iop, filename, (buf_idx+1)%2, bufs[buf_idx].cur_sec_nr, bufs[buf_idx].max_sec, cur_epoch);
                                                                     } else {
                                                                         // else start the next buf
                                                                         fetch_sector_remote((buf_idx + 1) % 2);
                                                                     }
                                                                 } else {
                                                                     fetch_sector_remote(buf_idx);
                                                                 }
                                                             }
                                                         }
                                                     }, filename, 1 << 18, sec_nr << 18, 0);
        // TODO: we are not sure the code below 100% run before the callback function
        bufs[buf_idx].ft_res_tab[sec_nr] = std::move(ft);
    }
}

ClientReadAhead::Status ClientReadAhead::status(const std::string& rpath, uint64_t offset, uint64_t size, char* dest) {
    uint64_t sector_bg = offset >> 18;
//            uint64_t sector_nr = (size >> 8) + (1-~(size&0xff)); // = upper(size/256)
    uint64_t sector_nr = 1; // currently fuse only send at most 128KB request
    mut.lock();
    if(rpath == bufs[0].file.second) {
        mut.unlock();
        auto st = fetch_cache(rpath, offset, size, dest);
        if (UNLIKE_SEQ_READ == st) {
            clear_buf(rpath);
        }
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

ClientReadAhead::Status ClientReadAhead::fetch_cache(const std::string& filename, uint64_t offset, uint64_t size, char* dest) {
    char* orig;
    uint64_t sector_bg = offset >> 18;
    for (int i= 0; i < 2; i++) {
        bufs[i].mut.lock();
        Status st = bufs[i].lookup(filename, sector_bg);
        if (st == NOT_FOUND) {
            bufs[i].mut.unlock();
            continue;
        } else if (st == IN_BUFFER) {
            orig = bufs[i].sector_addr(sector_bg) + (offset & 0x3ffff);
            if (orig + size > bufs[i].sector_addr(bufs[i].sec_bg + bufs[i].max_sec)) {
                bufs[i].mut.unlock();
                return UNLIKE_SEQ_READ;
            }
            std::memcpy(dest, orig, min(size, bufs[i].sec_bufferd[sector_bg] - (offset & 0x3ffff)));
            bufs[i].cursor = sector_bg > bufs[i].cursor ? sector_bg : bufs[i].cursor;
            bufs[i].mut.unlock();
            return IN_BUFFER;
        } else if (st == ON_LINK) {
            // wait until fulfilled
            bufs[i].mut.unlock();
            if (wait_sector(i, sector_bg)) {
                bufs[i].mut.lock();
                orig = bufs[i].sector_addr(sector_bg) + (offset & 0x3ffff);
                if (orig + size > bufs[i].sector_addr(bufs[i].sec_bg + bufs[i].max_sec)) {
                    bufs[i].mut.unlock();
                    return UNLIKE_SEQ_READ;
                }
                memcpy(dest, orig, min(size, bufs[i].sec_bufferd[sector_bg] - (offset & 0x3ffff)));
                bufs[i].cursor = sector_bg > bufs[i].cursor ? sector_bg : bufs[i].cursor;
                bufs[i].mut.unlock();
                return IN_BUFFER;
            } else {
                bufs[i].mut.unlock();
                return UNLIKE_SEQ_READ;
            }
        } else {
            bufs[i].mut.unlock();
            // impossible to reach here
            return UNLIKE_SEQ_READ;
        }
    }
}

// release the sector buffer before offset
bool ClientReadAhead::wait_sector(int buf_idx, uint64_t sec_nr) {
    // hold the live of this promise
    auto prom = std::make_shared<std::promise<bool>>();
    bufs[buf_idx].mut.lock();
    if(bufs[buf_idx].ft_waiting_tab.count(sec_nr) == 0) {
        bufs[buf_idx].prom_waiting_tab[sec_nr] = prom;
        bufs[buf_idx].ft_waiting_tab[sec_nr] = std::shared_future(bufs[buf_idx].prom_waiting_tab[sec_nr]->get_future());
    }
    // hold a new shared future
    std::shared_future<bool> sft (bufs[buf_idx].ft_waiting_tab[sec_nr]);
    bufs[buf_idx].mut.unlock();
    auto st = sft.wait_for(std::chrono::milliseconds(100));
    bufs[buf_idx].mut.lock();
    bool ret = (bufs[buf_idx].ft_waiting_tab.count(sec_nr) == 0 || st==std::future_status::ready && sft.get());
    if(bufs[buf_idx].ft_waiting_tab.count(sec_nr) > 0) {
        bufs[buf_idx].prom_waiting_tab.erase(sec_nr);
        bufs[buf_idx].ft_waiting_tab.erase(sec_nr);
    }
    bufs[buf_idx].mut.unlock();
    return ret;
}

void ClientReadAhead::clear_buf(const std::string& filepath) {
    if (filepath != bufs[0].file.second) {
        return;
    }
    uint64_t to_epoch = bufs[0].epoch++;
    // increase all buf's epoch to prevent reposonse callback
    for(int i = 0; i < 2; i++) {
        std::lock_guard<std::mutex> lock(bufs[i].mut);
        bufs[i].epoch = to_epoch;
        // wake up all the waiting reader
        for (auto &prom:bufs[i].prom_waiting_tab) {
            prom.second->set_value(false);
        }
        bufs[i].prom_waiting_tab.clear();
        bufs[i].ft_waiting_tab.clear();
        // clear other variable
        bufs[i].file.first.reset();
        bufs[i].file.second.clear();
        bufs[i].ft_res_tab.clear();
        bufs[i].sec_bufferd.clear();
        bufs[i].cur_sec_nr = 0;
        bufs[i].onlink = 0;
        bufs[i].sec_bg = 0;
        bufs[i].max_sec = 0;
        bufs[i].cursor = 0;
    }
}

bool ClientReadAhead::set_task(std::shared_ptr<IOProxyNode> iop, const std::string& filename,
        int buf_idx, uint64_t sec_bg, unsigned int sec_max, uint64_t epoch) {
    if (epoch == -1) {
        // means unlimited big epoch, we reduce it to next epoch
        epoch = bufs[buf_idx].epoch + 1;
    } else if (epoch < bufs[buf_idx].epoch) {
        // cannot set the task with smaller epoch
        return false;
    }
    // no need to speed up, already have this task
    {
        // if sec_bg smaller than any buf's sec_bg, then ignore it.
        if (filename == bufs[buf_idx].file.second &&
            (sec_bg < bufs[buf_idx].sec_bg + bufs[buf_idx].max_sec || bufs[buf_idx].cursor + 1 < bufs[buf_idx].sec_bg + bufs[buf_idx].max_sec)) {
             return false;
        }
        // choose a buf to use! we choose one with smaller sec_bg
//        int buf_idx = 0, smallest_sec = bufs[0].sec_bg;
//        for (int i = 1; i < 2; i++) {
//            if (bufs[i].sec_bg < smallest_sec) {
//                smallest_sec = bufs[i].sec_bg;
//                buf_idx = i;
//            }
//        }
        // set task on the choosen buf
        std::lock_guard<std::mutex> lock(bufs[buf_idx].mut);
        bufs[buf_idx].ft_res_tab.clear();
        // clear the promise in task
        for(auto& prom:bufs[buf_idx].prom_waiting_tab) {
            prom.second->set_value(false);
        }
        bufs[buf_idx].epoch = epoch;
        bufs[buf_idx].prom_waiting_tab.clear();
        bufs[buf_idx].ft_waiting_tab.clear();
        bufs[buf_idx].sec_bufferd.clear();
        bufs[buf_idx].sec_bg = sec_bg;
        bufs[buf_idx].max_sec = sec_max;
        bufs[buf_idx].buf = (char *) realloc(bufs[buf_idx].buf, sec_max << 18);
        bufs[buf_idx].cur_sec_nr = sec_bg;
        bufs[buf_idx].onlink = 0;
        bufs[buf_idx].file.first = iop;
        bufs[buf_idx].file.second = filename;
        bufs[buf_idx].cursor = 0;
        fetch_sector_remote(buf_idx);
    }
    dout(15) << "readahead task: " << filename << " sector: " << sec_bg << " at buf: "<< buf_idx << dendl;
    return true;
}