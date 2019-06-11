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
#include "client/ipc_struct.h"
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
        ipcServer.set_callback_func([](IPCMessage msg)-> std::string {
            char tmp[IPCMessage::max_body_length] = {0};
            std::memcpy(tmp, msg.body(), msg.body_length());
            std::cout << "Server端输出：" << tmp << endl;
            return "";
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
    // EXPECT_TRUE(send());
    try {
        IPCClient ipcClient("192.168.5.222", 6666);
        ipcClient.set_callback_func([&](IPCMessage msg)->void {
            // TODO:客户端输出服务端发送来的消息
            char tmp[IPCMessage::max_body_length] = {0};
            std::memcpy(tmp, msg.body(), msg.body_length());
            std::cout << "执行结果：" << tmp << endl;
        });
        ipcClient.run(); // 不用调用stop 函数；
        std::cout << "正在执行命令..." << endl;
        // TODO：构造命令行参数，并进行发送；
        char* demo[13] = {const_cast<char *>("spacerename"), const_cast<char *>("--ip"), const_cast<char *>("192.168.10.219"),
                          const_cast<char *>("-p"), const_cast<char *>("34779"), const_cast<char *>("--zonename"),
                          const_cast<char *>("compute-zonetest2"), const_cast<char *>("--id"), const_cast<char *>("000"),
                          const_cast<char *>("-o"), const_cast<char *>(""), const_cast<char *>("-n"),
                          const_cast<char *>("compute3")};
        int count = 13;
        CmdlineParameters cmdlineParameters(13, demo); //  初始化参数
        auto msg = IPCMessage::make_message_by_charstring(cmdlineParameters.serialize().c_str());
        ipcClient.write(*msg); // 传递一个消息；
        sleep(1); // TODO: 等待客户端返回结果
        ipcClient.stop();
    } catch (exception &e) {
        cout << e.what() << endl;
    }
}

// 测试接收
TEST(IPC_Test, Recv) {
    // EXPECT_TRUE(recv());
}


// 测试接收
TEST(IPC_Test, client) {

    EXPECT_TRUE(true);
}