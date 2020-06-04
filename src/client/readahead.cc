//
// Created by miller on 10/31/19.
// Modified by mark on 3/18/20
//

#include "client/readahead.h"
#include "client/client.h"
#include "client/msg_mod.h"

using namespace gvds;
using namespace std;

void ClientReadAhead::start() {
    auto _config = HvsContext::get_context()->_config;
    readahead = _config->get<int>("client.readahead").value_or(10);
    max_onlink = _config->get<int>("client.onlink").value_or(64);
}

void ClientReadAhead::stop() {}

void ClientReadAhead::fetch_sector_remote(BufIter bufIter) {
    std::shared_ptr<IOProxyNode> iop =bufIter->file.second;
    std::string filename = bufIter->file.first;


    auto oper = client->rpc->get_operator(iop, 100);
//    auto rpcc = client->rpc->rpc_channel(iop, false, 100);
    // the callback function could be used to trigger some event
    while (bufIter->onlink < max_onlink && bufIter->cur_sec_nr < bufIter->sec_bg + bufIter->max_sec) {
        bufIter->onlink++;
        uint64_t sec_nr = bufIter->cur_sec_nr++, cur_epoch = bufIter->epoch;
        std::function<void()> callback = [this, sec_nr, iop, filename, bufIter, cur_epoch]() {
            bufIter->mut.lock();
            if (bufIter->epoch != cur_epoch) {
                // out-of-date response, not expected
                bufIter->mut.unlock();
                return;
            } else {
                bufIter->onlink--;
                // fetch the response data from rpclib's future
                std::future<OpReply*> ft_cur;
                if (bufIter->ft_res_tab.count(sec_nr) > 0) {
                    ft_cur = std::move(bufIter->ft_res_tab[sec_nr]);
                   bufIter->ft_res_tab.erase(sec_nr);
                } else {
                    // if future not exsits, means new task has been set
                    bufIter->mut.unlock();
                    return;
                }
                bufIter->mut.unlock();

                // this callback function may be miss called even if there is a timeout
                auto st = ft_cur.wait_for(chrono::milliseconds(100));
                if (st != future_status::ready) return;
                auto reply = ft_cur.get();
//                auto retbuf = obj_hd.as<ioproxy_rpc_buffer>();
                if (reply->err_code() < 0) {
                    // stat failed on remote server
                    return;
                }

                // copy the data from response to buffer
                bufIter->mut.lock();
                if (filename != bufIter->file.first) {
                // cache is dirty
                    bufIter->mut.unlock();
                    return;
                }
                memcpy(bufIter->sector_addr(sec_nr), reply->data().c_str(),
                       reply->err_code());
                bufIter->mut.lock();
                bufIter->sec_buffered[sec_nr] = reply->err_code();
                // if there are someone who waiting for this sector, wake it up
                if (bufIter->prom_waiting_tab.count(sec_nr) > 0) {
                    bufIter->prom_waiting_tab[sec_nr]->set_value(
                            reply->err_code() >= 0);
                    bufIter->prom_waiting_tab.erase(sec_nr);
                }
                bufIter->mut.unlock();
                dout(READAHEAD_DEBUG_LEVEL) << "prefetch: " << filename << " sector: "
                                            << sec_nr
                                            << " size:" << reply->err_code()<< dendl;
            }
        };

        OpRequest request;
        request.set_type(OpType::read);
        request.mutable_io_param()->set_offset(sec_nr << 17);
        request.mutable_io_param()->set_size( 1 << 17);
        request.set_type(OpType::read);
        request.set_filepath(filename);

        auto ft = oper->SubmitAsync(request, callback);
//        auto ft = rpcc->_client->async_call_callback("ioproxy_read",
//                                                     , filename, 1 << 18, sec_nr << 18, 0);
        // TODO: we are not sure the code below 100% run before the callback function
        bufIter->ft_res_tab[sec_nr] = std::move(ft);
    }
}

ClientReadAhead::Status ClientReadAhead::status(const std::string& rpath, uint64_t offset, uint64_t size, char* dest, uint64_t& read_size) {
    uint64_t sec_nr = offset >> 17;
    bool alignment = ((sec_nr + 1) << 17 == (offset + size));
    mut.lock();
    auto lruIndex = lruList.find(rpath);
     // 文件不存在于缓存列表中
    if (lruIndex == lruList.end()) {
        if(!alignment) {
            mut.unlock();
            return NOT_FOUND;
        }
        // LRU排序
        if (lruList.full()) {
            auto last_lruIter = lruList.expire();
            clear_buf(last_lruIter->data);
            lruList.touch(rpath, last_lruIter->data);
        } else {
            lruList.touch(rpath, bufs.emplace(bufs.end()));
        }
        mut.unlock();
        return READAHEAD;
    } else {
        // 文件存在于缓存列表中
        auto st = lruIndex->data->lookup(sec_nr);
        if ( st == IN_BUFFER || st == ON_LINK) {
            lruIndex->data->mut.lock();
            mut.unlock();
            auto ret = fetch_cache(lruIndex->data, offset, size, dest, read_size);
            if (lruIndex->data->hit_trigger(sec_nr))
                append_prefetch(lruIndex->data);
            lruIndex->data->mut.unlock();
            return ret;
        } else {
            mut.unlock();
            if (alignment) return READAHEAD;
            else return NOT_FOUND;
        }
    }
}

ClientReadAhead::Status ClientReadAhead::fetch_cache(BufIter bufIter, uint64_t offset, uint64_t size, char *dest, uint64_t &read_size) {
    uint64_t sec_nr = offset >> 17;
    auto st = bufIter->lookup(sec_nr);
    switch (st)
    {
    case ON_LINK:
    {
        Status wait_st = wait_sector(bufIter, sec_nr);
        if (wait_st == NOT_FOUND)
        {
            st = wait_st;
            break;
        }
        else if (wait_st == ON_LINK)
            break;
    }
    case IN_BUFFER:
        char *orig = bufIter->sector_addr(sec_nr) + (offset & 0x1ffff);
        read_size = min(size, bufIter->sec_buffered[sec_nr] - (offset & 0x1ffff));
        std::memcpy(dest, orig, read_size);
        st = IN_BUFFER;
        break;
    }
    return st;
}

// release the sector buffer before offset
ClientReadAhead::Status ClientReadAhead::wait_sector(BufIter bufIter, uint64_t sec_nr) {
    // hold the live of this promise
    auto prom = std::make_shared<std::promise<bool>>();
    if(bufIter->ft_waiting_tab.count(sec_nr) == 0) {
        bufIter->prom_waiting_tab[sec_nr] = prom;
        bufIter->ft_waiting_tab[sec_nr] = std::shared_future(bufIter->prom_waiting_tab[sec_nr]->get_future());
    }
    // hold a new shared future
    std::shared_future<bool> sft (bufIter->ft_waiting_tab[sec_nr]);
    uint64_t cur_epoch = bufIter->epoch;
    bufIter->mut.unlock();
    dout(READAHEAD_DEBUG_LEVEL) << "readahead wait task sector: " << sec_nr << " at buf: "<< bufIter->file.first << dendl;
    auto st = sft.wait_for(std::chrono::milliseconds(1000));
    bufIter->mut.lock();
    if (bufIter->epoch != cur_epoch )
        return NOT_FOUND;
    bool ret = (bufIter->ft_waiting_tab.count(sec_nr) == 0 || st==std::future_status::ready && sft.get());
    if(bufIter->ft_waiting_tab.count(sec_nr) > 0) {
        bufIter->prom_waiting_tab.erase(sec_nr);
        bufIter->ft_waiting_tab.erase(sec_nr);
    }
    return ret ? IN_BUFFER : ON_LINK;
}

// 清除指定缓存控制块
void ClientReadAhead::clear_buf(BufIter bufIter) {
    bufIter->epoch++;
    for (auto &prom:bufIter->prom_waiting_tab) {
        prom.second->set_value(false);
    }
    bufIter->prom_waiting_tab.clear();
    bufIter->ft_waiting_tab.clear();
    // clear other variable
    bufIter->file.second.reset();
    bufIter->file.first.clear();
    bufIter->ft_res_tab.clear();
    bufIter->sec_buffered.clear();
    bufIter->sec_bg = 0;
    bufIter->sec_trigger = 0;
    bufIter->cur_sec_nr = 0;
    bufIter->onlink = 0;
    bufIter->max_sec = 0;
}

void ClientReadAhead::clear_buf(const string& rpath) {
    mut.lock();
    if (lruList.find(rpath) == lruList.end()) {
        mut.unlock();
        return;
    }
    auto bufIter = lruList.find(rpath)->data;
    if (bufIter->file.first.empty())
    {
        mut.unlock();
        return;
    }
    std::lock_guard<std::mutex> lock(bufIter->mut);
    mut.unlock();
    clear_buf(bufIter);
}

void ClientReadAhead::append_prefetch(BufIter bufIter) {
    // buffer at max length
    if (bufIter->sec_trigger == bufIter->sec_bg + readahead - 1 && bufIter->lookup(bufIter->sec_trigger) == IN_BUFFER){
        bufIter->ft_res_tab.clear();
        // clear the promise in task
        for(auto& prom:bufIter->prom_waiting_tab) {
            prom.second->set_value(false);
        }
        bufIter->epoch++;
        bufIter->prom_waiting_tab.clear();
        bufIter->ft_waiting_tab.clear();
        bufIter->sec_buffered.clear();
        bufIter->sec_bg = bufIter->sec_trigger + 1;
        bufIter->cur_sec_nr = bufIter->sec_bg;
        bufIter->sec_trigger = bufIter->sec_bg + 1;
        bufIter->max_sec = 4;
        bufIter->buf = (char *)realloc(bufIter->buf, bufIter->max_sec << 17);
        bufIter->onlink = 0;
    } else if(bufIter->sec_trigger != bufIter->sec_bg + readahead -1) {
        bufIter->max_sec = min(bufIter->max_sec + 4, readahead);
        bufIter->sec_trigger = min(bufIter->sec_trigger + 4, (unsigned int) bufIter->sec_bg + readahead - 1);
        bufIter->buf = (char *)realloc(bufIter->buf, bufIter->max_sec << 17);
    } else {
        return;
    }
    fetch_sector_remote(bufIter);
}

bool ClientReadAhead::set_task(std::shared_ptr<IOProxyNode> iop, const std::string& filename,
            uint64_t sec_bg, unsigned int sec_max, uint64_t epoch) {
    mut.lock();
    if (lruList.find(filename) == lruList.end()) {
        mut.unlock();
        return false;
    }
    auto bufIter = lruList.find(filename)->data;
    if (epoch == -1) {
        // means unlimited big epoch, we reduce it to next epoch
        epoch = bufIter->epoch + 1;
    } else if (epoch < bufIter->epoch) {
        // cannot set the task with smaller epoch
        return false;
    }
    // no need to speed up, already have this task
    {
        // choose a buf to use! we choose one with smaller sec_bg
//        int buf_idx = 0, smallest_sec = bufs[0].sec_bg;
//        for (int i = 1; i < 2; i++) {
//            if (bufs[i].sec_bg < smallest_sec) {
//                smallest_sec = bufs[i].sec_bg;
//                buf_idx = i;
//            }
//        }
        // set task on the choosen buf
        std::lock_guard<std::mutex> lock(bufIter->mut);
        // if sec_bg smaller than any buf's sec_bg, then ignore it.
        if (filename == bufIter->file.first && (
                sec_bg < bufIter->sec_bg + bufIter->max_sec && sec_bg >= bufIter->sec_bg)) {
            return false;
        }
        bufIter->ft_res_tab.clear();
        // clear the promise in task
        for(auto& prom:bufIter->prom_waiting_tab) {
            prom.second->set_value(false);
        }
        bufIter->epoch = epoch;
        bufIter->prom_waiting_tab.clear();
        bufIter->ft_waiting_tab.clear();
        bufIter->sec_buffered.clear();
        bufIter->sec_bg = sec_bg;
        bufIter->sec_trigger = sec_bg + 1;
        bufIter->max_sec = sec_max;
        bufIter->buf = (char *)realloc(bufIter->buf, sec_max << 17);
        bufIter->cur_sec_nr = sec_bg;
        bufIter->onlink = 0;
        bufIter->file.first = filename;
        bufIter->file.second = iop;
        fetch_sector_remote(bufIter);
    }
    dout(READAHEAD_DEBUG_LEVEL) << "readahead task: " << filename << " sector: " << sec_bg << " at buf: "<< bufIter->file.first << dendl;
    return true;
}