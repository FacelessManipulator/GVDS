/**
 * Created by Yaowen Xu on 2019-03-20.
 * 作者: Yaowen Xu
 * 时间: 2019-03-20
 * 工程: HVSONE
 * 作者单位: 北京航空航天大学计算机学院-系统结构研究所
 */

#include "gtest/gtest.h"
#include "ipc/IPCServer.hpp"
#include "ipc/IPCClient.h"
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
using namespace std;
using namespace hvs;

bool send() {
    cout << endl;
    try {
        IPCClient ipcClient("192.168.5.222", 6666);
        ipcClient.set_callback_func([&](IPCMessage msg)->void {
            // TODO:客户端输出服务端发送来的消息
            char tmp[IPCMessage::max_body_length] = {0};
            std::memcpy(tmp, msg.body(), msg.body_length());
            std::cout << "Client端输出：" << tmp << endl;
        });
        ipcClient.run(); // 不用调用stop 函数；
        std::cout << "客户端已经运行！" << endl;
        ipcClient.test(); // TODO:进行测试客户端和服务端间的连接的情况
        ipcClient.stop();
    } catch (exception &e) {
        cout << e.what() << endl;
    }
    return true;
}

bool recv() {
    cout << endl;
    try {
        IPCServer ipcServer(6666);
        ipcServer.set_callback_func([](IPCMessage msg)-> bool{
            char tmp[IPCMessage::max_body_length] = {0};
            std::memcpy(tmp, msg.body(), msg.body_length());
            std::cout << "Server端输出：" << tmp << endl;
            return true;
        });
        ipcServer.run(); // 启动 IPCServer 进行运行
        cout << "服务端已经运行！" << endl;
    } catch (exception &e){
        cout << e.what() << endl;
    }
    return true;
}

// 测试发送
TEST(IPC_Test, Send) {
    EXPECT_TRUE(send());
}

// 测试接收
TEST(IPC_Test, Recv) {
    EXPECT_TRUE(recv());
}

