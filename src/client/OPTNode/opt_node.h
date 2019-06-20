#ifndef _OPTNODE_H_
#define _OPTNODE_H_

#include <thread>
#include "client/client.h"
#include "common/centerinfo.h"

#include <iostream>
#include <vector>
#include <mutex>




namespace hvs{

typedef std::pair<std::string, double> PAIR;

struct struct_Node{
	std::string location;
	std::string ip_addr;
	std::string port;
};

struct CmpByValue {
  bool operator()(const PAIR& lhs, const PAIR& rhs) {
    return lhs.second < rhs.second;
  }
};

class SelectNode: public ClientModule, public Thread{
public:
    SelectNode (const char* name, Client* cli) : ClientModule(name, cli), m_stop(true) {
        std::cout << "start: SelectNode构造函数" << std::endl;
        isThread = true;  //TODO 这是什么意思？
    }
    ~SelectNode(){};

    //设置缓存初始值 [目前在构造函数实现]
    //void setOPT_init();
    
    //提供对外接口
    //1、获取管理节点IP信息
    std::string getCenterInfo(); //向客户端其他模块提供结果

    //2、获取最优节点选择策略结果
    std::vector<struct_Node> getNode(int choice);

    //3、获取centerName 对应的 centerid
    std::string getmapIdName(std::string centerName);


    void mutex_lock(); //因为mtx是私有，只能定义成员函数来操作
    void mutex_unlock();

    //buf_delay  1
    void setNode_delay(struct_Node &mynode);
    void erase_v_delay();

    //buf_area 2
    void setNode_area(struct_Node &mynode);
    void erase_v_area();
    
    

private:
    double subgetRTT_ping(std::string hostOrIp);
    double subgetRTT_rest(std::string hostOrIp, int port);
    double subgetRTT(std::string hostOrIp, int type = 1, int port = 9090);
    int getRTT(std::map<std::string, double> &mymap); //获取RTT
    void write_rtt(std::vector<PAIR> &myvec); //写缓存

    void getCenterInformation(); //向服务端获取center_Information


private:
    virtual void start() override;
    virtual void stop() override;
    void* entry() override;

    friend class Client;

private:
    static std::vector<struct_Node> buf_delay;   //static因为多个SelectNode对象需要共享同一个 缓冲区
    static std::vector<struct_Node> buf_area;
    static std::mutex mtx;  //static因为多个SelectNode对象需要使用同一个(共享)mtx  

    bool m_stop;

    std::string supercomputing_A = "Beijing"; 
    std::string supercomputing_B = "Shanghai"; 
    std::string supercomputing_C = "Guangzhou";
    std::string supercomputing_D = "Changsha";
    std::string supercomputing_E = "Jinan";

    std::string center_Information = "nothing"; //TODO设置成全局变量？？  是否设置成static
};


}//namespace hvs
#endif