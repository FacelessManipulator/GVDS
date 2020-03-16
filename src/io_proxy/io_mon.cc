#include "io_proxy/io_mon.h"
#include "io_proxy.h"
#include "msg/rpc.h"
#include <unistd.h>

using namespace std;
using namespace gvds;

void IOMonitor::start() {
    // invoke the monitor function
    iop->_rpc->native()->invoke_monitor([this](std::string&& ip, int po_, std::size_t sz){
        this->invoke_monitor(ip, po_, sz);
    });
    m_stop = false;
    create("IOProxy_Monitor");
}

void IOMonitor::invoke_monitor(std::string ip, int po_, std::size_t sz) {
    // TODO: We assume every ip start with 172 is localnet address, but this is not accurate.
    // The precise range is 10.x.x.x, 172.16.0.0-172.31.255.255, 192.168.x.x
    auto addr = boost::asio::ip::address::from_string(ip);
    bool islocal = false;
    if((addr>=LOCALNET_ADDR_10_START && addr<=LOCALNET_ADDR_10_END) ||
       (addr>=LOCALNET_ADDR_172_START && addr<=LOCALNET_ADDR_172_END) ||
       (addr>=LOCALNET_ADDR_192_START && addr<=LOCALNET_ADDR_192_END)
            ) {
        islocal = true;
    }
    switch (po_) {
        case MONITOR_INVOKE_POINT::READ:
            if(islocal)
                ms->byte_read_local.second += sz;
            else
                ms->byte_read_remote.second += sz;
            break;
        case MONITOR_INVOKE_POINT::WRITE:
            if(islocal)
                ms->byte_write_local.second += sz;
            else
                ms->byte_write_remote.second += sz;
            break;
        default:
            break;
    }
}

void IOMonitor::stop() {
    m_stop = true;
    join();
}

void* IOMonitor::entry() {
    while(!m_stop) {
        // we don't need lock here cuz inconsistency could be tolerant
        ms->flush_speed(FRESH_MILLISECONDS);
        usleep(FRESH_MILLISECONDS*1000);
    }
}