/**
 * Created by Yaowen Xu on 2019-03-20.
 * 作者: Yaowen Xu
 * 时间: 2019-03-20
 * 工程: HVSONE
 * 作者单位: 北京航空航天大学计算机学院-系统结构研究所
 */

// 实现了 socket 通信操作

#include "IPCClient.h"
using namespace hvs;
using boost::asio::ip::tcp;

bool IPCClient::run() {
    //新开一个线程，进行发送消息；
    try {
        sp_context_thread = std::make_shared<std::thread>([this](){ io_context_.run(); });
    }catch (std::exception& e){
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return true;
}

bool IPCClient::stop() {
    try {
        //sleep(3); // 增加三秒钟的延迟，保证所有消息都成功；
        io_context_.stop();
        sp_context_thread->join(); // 等待client 自身线程结束；
        boost::asio::post(io_context_, [this]() { socket_.close(); }); // 进行关闭socket
    }catch (std::exception& e){
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return true;
}

IPCClient::~IPCClient() {
    // 调用析构函数处理函数值；
    // stop();
}

IPCClient::IPCClient():socket_(io_context_) { // 进程客户端
    // 默认构造函数端口为6666
    tcp::resolver resolver(io_context_);
    auto  endpoints = resolver.resolve("127.0.0.1", "6666");
    do_connect(endpoints); // 处理端点信息
}

IPCClient::IPCClient(std::string ip, int port):socket_(io_context_){
    tcp::resolver resolver(io_context_);
    auto  endpoints = resolver.resolve(ip, std::to_string(port));
    do_connect(endpoints); // 处理端点信息
}

void IPCClient::do_connect(const boost::asio::ip::tcp::resolver::results_type &endpoints) {
    boost::asio::async_connect(socket_, endpoints,
                               [this](boost::system::error_code ec, tcp::endpoint)
                               {
                                   if (!ec)
                                   {
                                       do_read_header();
                                   }
                               });
}

void IPCClient::do_read_header() {
    boost::asio::async_read(socket_,
                            boost::asio::buffer(read_msg_.data(), IPCMessage::header_length),
                            [this](boost::system::error_code ec, std::size_t /*length*/)
                            {
                                if (!ec && read_msg_.decode_header())
                                {
                                    do_read_body();
                                }
                                else
                                {
                                    socket_.close();
                                }
                            });
}

void IPCClient::do_read_body() {
    boost::asio::async_read(socket_,
                            boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                            [this](boost::system::error_code ec, std::size_t /*length*/)
                            {
                                if (!ec)
                                {
                                    // 消息接收完毕，可以开始处理。
                                    push_to_return_msgs(read_msg_);
                                    do_read_header();
                                }
                                else
                                {
                                    socket_.close();
                                }
                            });
}

void IPCClient::do_write() { // 执行对 ipc 写操作，其中有一个写队列
    boost::asio::async_write(socket_,
                             boost::asio::buffer(write_msgs_.front().data(),
                                                 write_msgs_.front().length()),
                             [this](boost::system::error_code ec, std::size_t /*length*/)
                             {
                                 if (!ec)
                                 {
                                     write_msgs_.pop_front();
                                     if (!write_msgs_.empty())
                                     {
                                         do_write();
                                     }
                                 }
                                 else
                                 {
                                     socket_.close();
                                 }
                             });
}

void IPCClient::write(const IPCMessage &msg) {
    boost::asio::post(io_context_,
                      [this, msg]()
                      {
                          bool write_in_progress = !write_msgs_.empty();
                          write_msgs_.push_back(msg);
                          if (!write_in_progress)
                          {
                              do_write();
                          }
                      });
}

bool IPCClient::test() {
    for (int i = 0; i < 10; ++i) {
        sleep(1); // 进行等待一秒钟，进行发送消息；
        auto tmp = IPCMessage::make_message_by_charstring("Yaoxu");
        write(*tmp); // 传递一个消息；
    }
    return true;
}

void IPCClient::push_to_return_msgs(const IPCMessage msg) {
    bool job_in_progress = !return_msgs_.empty();
    return_msgs_.push_back(msg);
    if (!job_in_progress)
    {
        do_jobs();
    }
}

void IPCClient::do_jobs() {
    /*
     *  在这里处理函数逻辑；std::cout << return_msgs_.front().body() << std::endl;
     */
    // 调用回调函数，处理每次逻辑；
    (*sp_process_func)(return_msgs_.front());
    return_msgs_.pop_front();
    if (!return_msgs_.empty())
    {
        do_jobs();
    }
}

void IPCClient::set_callback_func(std::function<void (IPCMessage)> func) {
    sp_process_func = std::make_shared<std::function<void (IPCMessage)>>(func);
}



