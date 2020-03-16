/**
 * Created by Yaowen Xu on 2019-03-20.
 * 作者: Yaowen Xu
 * 时间: 2019-03-20
 * 工程: GVDS
 * 作者单位: 北京航空航天大学计算机学院-系统结构研究所
 */

#ifndef GVDS_IPCCLIENT_H
#define GVDS_IPCCLIENT_H
#include <iostream>
#include <boost/asio.hpp>
#include "IPCMessage.hpp" // 引入消息格式
#include <cstdlib>
#include <deque>
#include <thread>
#include <string>

namespace gvds{
    typedef std::deque<IPCMessage> ipc_message_queue;

    class IPCClient {
    public:
        IPCClient();
        IPCClient(std::string ip, int port);
        ~IPCClient();
        void set_callback_func(std::function <void (IPCMessage)>);
        bool run();
        bool test();
        void write(const IPCMessage& msg);
        bool stop();
    private:
        void do_connect(const boost::asio::ip::tcp::resolver::results_type& endpoints); // 处理 connect 信息
        void do_read_header();
        void do_read_body();
        void do_write();
        void do_jobs();
        void push_to_return_msgs(const IPCMessage msg);

    private:
        boost::asio::io_context io_context_;
        boost::asio::ip::tcp::socket socket_;
        IPCMessage read_msg_;
        ipc_message_queue write_msgs_; // 写入的消息队列
        ipc_message_queue return_msgs_; // 返回的消息队列
        std::shared_ptr<std::thread> sp_context_thread; // 用来指向线程
        std::shared_ptr<std::function <void (IPCMessage)>>  sp_process_func; // 指向函数和lamda 表达式的智能指针
    };

    inline int GVDS_CLIENT_LISTENING_PORT() {
        // 6666 is the old version default client listening port.
        int port = 6666;
        try {
            port = std::stoi(std::getenv("GVDS_CLIENT_LISTEN_PORT"));
        } catch(std::exception e) {
            // if env variable not exists, use old default value for compatibility
            return 6666;
        }
        return port;
    }
}

#endif //GVDS_IPCCLIENT_H
