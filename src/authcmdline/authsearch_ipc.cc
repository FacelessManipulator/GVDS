#include <iostream>
#include <future>
#include <pistache/client.h>
#include "cmdline/CmdLineProxy.h"

// TODO: 添加的新头文件
// #include "client/clientuser/ClientUser_struct.h"
#include "client/clientuser/ClientAuth_struct.h"
#include "ipc/IPCClient.h"
#include "client/ipc_struct.h"




using namespace Pistache;
using namespace hvs;
using namespace std;
//bool GetZoneInfo(std::string ip, int port, std::string clientID);
/*
 * zonerename 命令行客户端
 */
//std::unordered_map<std::string, std::string> zonemap;


int main(int argc, char* argv[]){

    //权限查询
    char* demo1[7] = {const_cast<char *>("authsearch_ipc"), 
                       const_cast<char *>("--user"), const_cast<char *>("lbq-7")};
    char* demo2[2] = {const_cast<char *>("authsearch_ipc"), const_cast<char *>("--help")};

    // TODO: 提前准备的数据
    std::string username;//= "lbq-7";

    // TODO: 获取命令行信息
    CmdLineProxy commandline(argc, argv);
//    CmdLineProxy commandline(2, demo2);
    std::string cmdname = argv[0];
    // TODO：设置当前命令行解析函数
    commandline.cmd_desc_func_map[cmdname] =  [](std::shared_ptr<po::options_description> sp_cmdline_options)->void {
        po::options_description command("权限查询");
        command.add_options()
                ("user,u", po::value<std::string>(), "账户名")
                ;
        sp_cmdline_options->add(command); // 添加子模块命令行描述
    };
    // TODO： 解析命令行参数，进行赋值
    commandline.cmd_do_func_map[cmdname] =  [&](std::shared_ptr<po::variables_map> sp_variables_map)->void {
        if (sp_variables_map->count("user"))
        {
            username = (*sp_variables_map)["user"].as<std::string>();
        }
        
    };
    commandline.start(); //开始解析命令行参数

    //TODO :判断是否有参数，如果没有，则报错
    if (commandline.argc <= 1 ) {
        std::cerr << "请输入命令参数！" << std::endl;
        commandline.print_options();
        exit(-1);
    }

     try{
        std::promise<bool> prom;
        auto fu = prom.get_future();
        // TODO:  调用IPC 客户端 进行同行，并获取返回结果
        IPCClient ipcClient("127.0.0.1", 6666);
        ipcClient.set_callback_func([&](IPCMessage msg)->void {
            // 客户端输出服务端发送来的消息
            std::string ipcresult (msg.body(), msg.body_length());
            if(ipcresult == "fail"){
                std::cout << "Not Found Zone" << std::endl;
            }
            else if(ipcresult == "33"){
                std::cout << "Verification failed, access denied" << std::endl;
            }
            else if(ipcresult == "client_input_error"){
                std::cout << "账户名输入错误" << std::endl;
            }
            else{
                AuthSearch myauth;
                myauth.deserialize(ipcresult);

                // cout << myauth.hvsID << endl;
                vector<string>::iterator iter;
                for (iter = myauth.vec_ZoneID.begin(); iter != myauth.vec_ZoneID.end(); iter++){
                    cout << "区域id： " << *iter << endl;
                    cout << "区域名字: " << myauth.zoneName[*iter] << endl;
                    cout << "区域读权限： " << myauth.read[*iter] << endl;
                    cout << "区域写权限： " << myauth.write[*iter] << endl;
                    cout << "区域执行权限： "<< myauth.exe[*iter] << endl;  
                    cout << "身份： ";
                    if(myauth.isowner[*iter]=="1"){
                        cout << "区域拥有者" << endl;  
                        cout << "组-读权限：" << myauth.ownergroupR[*iter] << endl;
                        cout << "组-写权限： " << myauth.ownergroupW[*iter] << endl;
                        cout << "组-执行权限： " << myauth.ownergroupE[*iter] << endl;
                    }
                    else{
                        cout << "区域成员" << endl;  
                    }
                    cout << endl;
                }
            }
            prom.set_value(true);
        });
        ipcClient.run(); // 停止的时候调用stop 函数
        std::cout << "正在执行命令..." << std::endl;


         // TODO: 构造请求结构体，并发送；
        IPCreq ipcreq;
        ipcreq.cmdname = "authsearch"; //这里每个不一样
        
        ipcreq.accountName = username; //账户名
        

        // TODO: 发送
        auto msg = IPCMessage::make_message_by_charstring(ipcreq.serialize().c_str());
        ipcClient.write(*msg); // 传递一个消息；
        fu.get();
        // sleep(1); // TODO: 等待客户端返回结果
        ipcClient.stop();

    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }



}