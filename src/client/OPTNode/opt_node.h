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
        isThread = false;  //TODO 这是什么意思？
        
        getCenterInformation();//获取center_Information
        std::cout << "center_Information : " << center_Information << std::endl;
        if(center_Information!= "nothing"){
            std::cout << "设置opt缓存初值" << std::endl;
            CenterInfo mycenter;
            mycenter.deserialize(center_Information);    //b-1 s-2 g-3 c-4 j-5
            
            //读取配置文件，作为初值
            struct_Node local;
            std::vector<std::string>::iterator iter;
            for( iter = mycenter.centerID.begin(); iter!=mycenter.centerID.end(); iter++){

                local.location = mycenter.centerName[*iter];  //从centerInfo里获取
                local.ip_addr = mycenter.centerIP[*iter];
                local.port = mycenter.centerPort[*iter];
                buf_delay.push_back(local);
            }
        }
        else{
            getCenterInformation();//再次获取
        }
        std::cout << "end: SelectNode构造函数" << std::endl;
    }
    ~SelectNode(){};
    
    //expo
    std::vector<struct_Node> getNode(int choice);
    void mutex_lock(); //因为mtx是私有，只能定义成员函数来操作
    void mutex_unlock();

    //buf_delay  1
    void setNode_delay(struct_Node &mynode);
    void erase_v_delay();

    //buf_area 2
    void setNode_area(struct_Node &mynode);
    void erase_v_area();

private:
    double subgetRTT(std::string hostOrIp);
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

    std::string center_Information = "nothing";
};


}//namespace hvs
#endif