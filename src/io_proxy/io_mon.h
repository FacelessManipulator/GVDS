#pragma once
#include <atomic>
#include <string>
#include <functional>
#include <chrono>
#include "common/Thread.h"
#include "boost/asio/ip/address.hpp"
#include "monitor_counter.hpp"

namespace hvs {
    class IOProxy;
    class IOMonitor: public Thread {
    private:
        std::unique_ptr<MonitorSpeed> ms;
        int m_stop;
    public:
        const uint32_t FRESH_MILLISECONDS = 1000;
        enum MONITOR_INVOKE_POINT {
            READ = 0,
            WRITE
        };
    public:
        IOMonitor(IOProxy* ioProxy) :iop(ioProxy) {
            LOCALNET_ADDR_10_START.from_string("10.0.0.0");
            LOCALNET_ADDR_10_END.from_string("10.255.255.255");
            LOCALNET_ADDR_172_START.from_string("172.16.0.0");
            LOCALNET_ADDR_172_END.from_string("172.31.255.255");
            LOCALNET_ADDR_192_START.from_string("192.168.0.0");
            LOCALNET_ADDR_192_END.from_string("192.168.255.255");
            ms = std::make_unique<MonitorSpeed>();
        }
        void start();
        void* entry() override;
        void stop();
        MonitorSpeed* getSpeed() {return ms.get();}

    private:
        void invoke_monitor(std::string ip, int po_, std::size_t sz);
    private:
        IOProxy* iop;
        boost::asio::ip::address LOCALNET_ADDR_10_START;
        boost::asio::ip::address LOCALNET_ADDR_10_END;
        boost::asio::ip::address LOCALNET_ADDR_172_START;
        boost::asio::ip::address LOCALNET_ADDR_172_END;
        boost::asio::ip::address LOCALNET_ADDR_192_START;
        boost::asio::ip::address LOCALNET_ADDR_192_END;
    };
};