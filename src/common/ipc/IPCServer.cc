#include <utility>

/**
 * Created by Yaowen Xu on 2019-03-20.
 * 作者: Yaowen Xu
 * 时间: 2019-03-20
 * 工程: HVSONE
 * 作者单位: 北京航空航天大学计算机学院-系统结构研究所
 */

#include "IPCServer.hpp"
using namespace hvs;
using boost::asio::ip::tcp; // 单独使用 tcp 命名空间
typedef std::deque<IPCMessage> ipc_message_queue; // 声明一个消息队列

class IPCSession:public IPCTask, public std::enable_shared_from_this<IPCSession> {//允许从内部获取自己的智能指针
    class  IPCSessionWorker {
    public:
        IPCSessionWorker(IPCSession &session, std::shared_ptr<std::function <std::string (IPCMessage)>>  sp_func):
        session_(session),
        sp_process_func(std::move(sp_func))
        {
            //进行处理传进来的消息；
        }
        void add_task(const IPCMessage& msg){
            // 测试语句
            bool work_in_progress = !tasks_queue_.empty();
            tasks_queue_.push_back(msg);
            if (!work_in_progress){
                // 处理工作
                do_work();
            }
        }
        void do_work(){ // 进行重新开一个线程，进行处理工作; 进行重新开一个线程进行处理消息
            // TODO： 所有消息在这里进行调用回调函数, 处理！并返回成功与否
            std::string ret = (*sp_process_func)(tasks_queue_.front());
            session_.deal_with_msg(*(IPCMessage::make_message_by_charstring(ret.c_str())));// 返回执行结果！
            tasks_queue_.pop_front(); // 进行处理消息
        }
    private:
        IPCSession& session_;
        ipc_message_queue tasks_queue_; // 当前任务队列
        std::shared_ptr<std::function <std::string (IPCMessage)>>  sp_process_func;
    };
public:
    IPCSession(tcp::socket socket, std::shared_ptr<std::function <std::string (IPCMessage)>>  sp_func, IPCTaskCollection& task_collection):
    socket_(std::move(socket)),
    task_collection_(task_collection),
    session_worker_(*this, std::move(sp_func))
    {

    }

    ~IPCSession() override {
        std::cout << "Session-Client: " << addr << ":" << po << " closed!"<< std::endl; // 处理析构时的段错误
    }

    void start(){
        std::cout << "Session-Client: " << socket_.remote_endpoint().address() << ":" << socket_.remote_endpoint().port() << std::endl;
        addr =  socket_.remote_endpoint().address();
        po = socket_.remote_endpoint().port();
        task_collection_.join(shared_from_this());
        do_read_header(); // 处理消息头
    }

    void deal_with_msg(const IPCMessage& msg){
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress)
        {
            do_write();
        }
    }

private:
    void do_read_header() // 处理消息头
    {
        auto self(shared_from_this());
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.data(), IPCMessage::header_length),
                                [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                {
                                    if (!ec && read_msg_.decode_header())
                                    {
                                        do_read_body();
                                    }
                                    else
                                    {
                                        task_collection_.leave(shared_from_this());
                                    }
                                });
    }

    void do_read_body()
    {
        auto self(shared_from_this());
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                {
                                    if (!ec)
                                    {
                                        // 把当前消息交给了worker 处理，并开始接收下一个消息
                                        session_worker_.add_task(read_msg_); //
                                        do_read_header();
                                    }
                                    else
                                    {
                                        task_collection_.leave(shared_from_this());
                                    }
                                });
    }

    void do_write()
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(write_msgs_.front().data(),
                                                     write_msgs_.front().length()),
                                 [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                 {
                                     if (!ec)
                                     {
                                         write_msgs_.pop_front(); // 发送消息完毕，弹出写队列的第一个消息，判断消息队列是否为空，如果不为空，开始处理下一个消息。
                                         if (!write_msgs_.empty())
                                         {
                                             do_write();
                                         }
                                     }
                                     else
                                     {
                                         task_collection_.leave(shared_from_this()); // 离开当前任务集，表示session 已经中断。
                                     }
                                 });
    }

    IPCMessage read_msg_;
    IPCTaskCollection& task_collection_; // 展示当前已经连接的所有任务
    ipc_message_queue write_msgs_;
    tcp::socket socket_;
    boost::asio::ip::address addr;  // 添加此变量，避免了命令行提前退出导致的客户端段错误，从而导致hvs_client 崩溃
    unsigned short po; // 添加此变量，避免了命令行提前退出导致的客户端段错误，从而导致hvs_client 崩溃 此变量将在析构函数中再次被使用！
    IPCSessionWorker session_worker_;
};

IPCServer::IPCServer() {
    // 默认情况下，端口为 6666
    boost::asio::ip::tcp::resolver resolver(io_context_);
    tcp::endpoint endpoint(tcp::v4(), 6666);
    acceptor_ = std::make_shared<tcp::acceptor>(io_context_, endpoint);
    do_accept();
}

IPCServer::IPCServer(int port) {
    std::cout << "IPC listen: 0.0.0.0:" << port << std::endl;
    boost::asio::ip::tcp::resolver resolver(io_context_);
    tcp::endpoint endpoint(tcp::v4(), static_cast<unsigned short>(port));
    acceptor_ = std::make_shared<tcp::acceptor>(io_context_, endpoint);
    do_accept();
}


IPCServer::~IPCServer() {
    // stop();
}

bool IPCServer::run() {
    // 新开一个线程，执行接受操作；
    try {
        sp_context_thread = std::make_shared<std::thread>([this](){ io_context_.run(); });
    }catch (std::exception& e){
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return true;
}

bool IPCServer::stop() {
    // 上下文结束执行
    try {
        io_context_.stop();
        sp_context_thread->join(); // 等待server自身线程结束；
    }catch (std::exception& e){
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return true;
}

void IPCServer::do_accept() {
    acceptor_->async_accept(
            [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket){
                if(!ec){
                    std::make_shared<IPCSession>(std::move(socket), sp_process_func, task_collection_)->start(); // 把socket 传递给session 对象处理，并开始下一次接收
                }
                do_accept(); // 开始下一次异步接收，保持服务高并发。
            });
}

void IPCServer::set_callback_func(std::function<std::string (IPCMessage)> func) {
    // 传递回调指针；
    sp_process_func = std::make_shared<std::function<std::string (IPCMessage)>>(func);
}


