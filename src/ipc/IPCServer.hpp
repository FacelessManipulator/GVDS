/**
 * Created by Yaowen Xu on 2019-03-20.
 * 作者: Yaowen Xu
 * 时间: 2019-03-20
 * 工程: HVSONE
 * 作者单位: 北京航空航天大学计算机学院-系统结构研究所
 */

#ifndef HVSONE_IPCSERVER_H
#define HVSONE_IPCSERVER_H

#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/system/error_code.hpp>
#include <boost/bind/bind.hpp>
#include <memory>
#include <deque>
#include <set>
#include <utility>
#include <list>
#include <cstdlib>
#include <thread>
#include "IPCMessage.hpp"

namespace hvs{
    class IPCTask { // 每个Session都是一个Task
    public:
        virtual ~IPCTask() {}
        virtual void deal_with_msg(const IPCMessage& msg) = 0; // 声明一个纯虚函数，实现任务的处理
    };

    typedef std::shared_ptr<IPCTask> IPCTask_ptr; // 定义一个指向上述抽象类的智能指针。

    class IPCTaskCollection { // 知道全局Client 状态信息
    public:
        void join (IPCTask_ptr task){
            all_tasks_.insert(task); // 处理所有任务
        }

        void leave (IPCTask_ptr task){
            all_tasks_.erase(task);
        }

    private:
        std::set<IPCTask_ptr> all_tasks_;
    };

    class IPCServer {
    public:
        IPCServer();
        IPCServer(int port);
        ~IPCServer();
        void set_callback_func(std::function <std::string (IPCMessage)>);
        bool run();
        bool stop(); // 服务器结束函数，处理函数结束时的现场工作；
    private:
        void do_accept();
        boost::asio::io_context io_context_; // 服务端上下文
        std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_; // 服务端接收器
        IPCTaskCollection task_collection_;
        std::shared_ptr<std::thread> sp_context_thread;
        std::shared_ptr<std::function <std::string (IPCMessage)>>  sp_process_func; // 指向函数和lamda 表达式的智能指针
    };
}

#endif //HVSONE_IPCSERVER_H
