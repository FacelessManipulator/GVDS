#include <iostream>
#include <pthread.h>
#include <vector>
#include <unistd.h>
#include <unistd.h>
#include <utility> //pair
#include <algorithm>  //sort

#include <pistache/client.h>
#include <pistache/http.h>
#include <pistache/net.h>
#include <atomic>
#include <chrono>

#include "opt_node.h"
#include "myping.h"
#include "client/msg_mod.h"

using namespace Pistache;
using namespace Pistache::Http;
using namespace hvs;
using namespace std;

//mutex mtx_write

namespace hvs{


std::vector<struct_Node> SelectNode::buf_delay;
std::vector<struct_Node> SelectNode::buf_area;
std::mutex SelectNode::mtx;

void SelectNode::mutex_lock(){
    mtx.lock();
}

void SelectNode::mutex_unlock(){
    mtx.unlock();
}

vector<struct_Node> SelectNode::getNode(int choice){   //调用的时候判断下返回的vector.size()是否为0
    mutex_lock();
    std::vector<struct_Node> return_buf;

    if (choice == 1){
        for (int i=0; i< buf_delay.size(); i++){
             //cout <<"1:"<< buf_delay.at(i).location << " " << buf_delay.at(i).ip_addr << " " << buf_delay.at(i).port<< endl;
             return_buf.push_back(buf_delay.at(i));
        }
        mutex_unlock();
        return return_buf;
    }
    else if (choice == 2){
        return buf_area;
    }
    else{
        return buf_delay;
    }
}

string SelectNode::getCenterInfo(){
    return center_Information;
}

//delay
void SelectNode::setNode_delay(struct_Node &mynode){
    buf_delay.push_back(mynode);
}

void SelectNode::erase_v_delay(){
    buf_delay.clear();
}

//area
void SelectNode::setNode_area(struct_Node &mynode){
    buf_area.push_back(mynode);
}

void SelectNode::erase_v_area(){
    buf_area.clear();
}

void SelectNode::start(){
    m_stop = false;
    getCenterInformation();
    if(!center_Information.empty()){
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
    create("opt_node");
}

void SelectNode::stop(){
    m_stop = true;
}

// void SelectNode::setOPT_init(){
//     getCenterInformation();//获取center_Information
//     std::cout << "center_Information : " << center_Information << std::endl;

//         if(!center_Information.empty()){
//             std::cout << "设置opt缓存初值" << std::endl;
//             CenterInfo mycenter;
//             mycenter.deserialize(center_Information);    //b-1 s-2 g-3 c-4 j-5
            
//             //读取配置文件，作为初值
//             struct_Node local;
//             std::vector<std::string>::iterator iter;
//             for( iter = mycenter.centerID.begin(); iter!=mycenter.centerID.end(); iter++){

//                 local.location = mycenter.centerName[*iter];  //从centerInfo里获取
//                 local.ip_addr = mycenter.centerIP[*iter];
//                 local.port = mycenter.centerPort[*iter];
//                 buf_delay.push_back(local);
//             }
//         }
//         else{
//             getCenterInformation();//再次获取
//         }
// }

void* SelectNode::entry() {
    
    map<string, double> rankrtt;

    while(!m_stop) {
        rankrtt.clear();
        if (getRTT(rankrtt)==0){;// get 5个地点rtt
            vector<PAIR> myvec(rankrtt.begin(), rankrtt.end());
            sort(myvec.begin(), myvec.end(), CmpByValue()); //排序
            write_rtt(myvec);  //加锁，写入
        }
        std::this_thread::sleep_for(std::chrono::seconds(100));
        getCenterInformation();
    }
}





//线程功能函数
// int SelectNode::getRTT_notuse(map<string, double> &mymap){

//     //string a = "www.baidu.com";
//     if(!center_Information.empty()){
//         CenterInfo mycenter;
//         mycenter.deserialize(center_Information); 
//         //for 改成for

//         double rtt_beijing = subgetRTT(mycenter.centerIP["1"], 2,
//             atoi(mycenter.centerPort["1"].c_str()));
//         double rtt_shanghai = subgetRTT(mycenter.centerIP["2"], 2,
//             atoi(mycenter.centerPort["2"].c_str()));
//         double rtt_guangzhou = subgetRTT(mycenter.centerIP["3"], 2, 
//             atoi(mycenter.centerPort["3"].c_str()));
//         double rtt_changsha = subgetRTT(mycenter.centerIP["4"], 2, 
//             atoi(mycenter.centerPort["4"].c_str()));
//         double rtt_jinan = subgetRTT(mycenter.centerIP["5"], 2,
//          atoi(mycenter.centerPort["5"].c_str()));

//         mymap[supercomputing_A] = rtt_beijing;
//         mymap[supercomputing_B] = rtt_shanghai;
//         mymap[supercomputing_C] = rtt_guangzhou;
//         mymap[supercomputing_D] = rtt_changsha;
//         mymap[supercomputing_E] = rtt_jinan;

//         return 0;
//     }
//     return -1;
// }


int SelectNode::getRTT(map<string, double> &mymap){
    if(!center_Information.empty()){
        CenterInfo mycenter;
        mycenter.deserialize(center_Information); 
        //for 改成for

        std::vector<std::string>::iterator iter;
        for(iter = mycenter.centerID.begin(); iter != mycenter.centerID.end(); iter++){
            double location_rtt = subgetRTT(mycenter.centerIP[*iter], 2, 
                                            atoi(mycenter.centerPort[*iter].c_str()));

            mymap[mycenter.centerName[*iter]] = location_rtt;
        }

        return 0;
    }
    return -1;
}

//获取给定ip的实验时延
double SelectNode::subgetRTT(std::string hostOrIp, int type, int port) {
    if(type == 1) {
        return subgetRTT_ping(hostOrIp);
    } else if(type == 2) {
        return subgetRTT_rest(hostOrIp, port);
    }
}

double SelectNode::subgetRTT_rest(string hostOrIp, int port) {
  char url[64];
  snprintf(url, 64, "http://%s:%d", hostOrIp.c_str(), port);
  auto start = chrono::steady_clock::now();
  string response = client->rpc->get_request(url, "/manager");
  auto end = chrono::steady_clock::now();

  return chrono::duration_cast<chrono::microseconds>(end - start).count();
}

double SelectNode::subgetRTT_ping(string hostOrIp){
    
    //string hostOrIp = "www.baidu.com";
    int nsend = 0, nreceived = 0;
    bool ret;
    PingResult pingResult;
    Ping ping = Ping();
    double rtt = 0;

    int forvalue = 4;
    for (int count = 1; count <= forvalue; count++){
        ret = ping.ping(hostOrIp, 1, pingResult);
        if (count == 1){
            //（1/2） 展示工作量时输出
            // cout << "PING " << hostOrIp << "(" << pingResult.ip.c_str() << "):" << pingResult.dataLen << " bytes data in ICMP packets." << endl;
        }
        if (!ret){
            //printf("%s\n", pingResult.error.c_str());
            cout << pingResult.error.c_str() << endl;
            break;
        }
        //showPingResult(pingResult);
        nsend += pingResult.nsend;
        nreceived += pingResult.nreceived;
        rtt += pingResult.icmpEchoReplys.at(0).rtt; 
    }
    if (ret){
         //（2/2）展示工作量时输出
        // cout << nsend << "packets transmitted, " << nreceived << "received ,"<< ((nsend - nreceived) / nsend * 100) << "lost." << endl;
        rtt = rtt/forvalue * 1.0;
    }
    else{
        cout << "fail, rtt = 10000.0;" << endl;
        rtt = 10000.0;
    }
    return rtt;

}


//输入排序， 写入缓存
void SelectNode::write_rtt(vector<PAIR> &myvec){
    
    if(!center_Information.empty()){
        CenterInfo mycenter;
        mycenter.deserialize(center_Information);    //b-1 s-2 g-3 c-4 j-5

        mutex_lock(); //cout<<"wirte lock"<<endl; //展示工作量时输出
        erase_v_delay();
        for (int i = 0; i != myvec.size(); ++i) {
            string id = getmapIdName(myvec[i].first);   // myvec[i].first是hostCenterName   是rtt
            
            struct_Node myNode;
            myNode.location = mycenter.centerName[id]; 
            myNode.ip_addr = mycenter.centerIP[id];
            myNode.port = mycenter.centerPort[id];
            setNode_delay(myNode);

            // cout << myvec[i].first << " : " << myvec[i].second << endl; //展示工作量时输出
            // if (myvec[i].first.compare(supercomputing_A) == 0)
        }
        mutex_unlock();
    }
    else{
         getCenterInformation();
    }
    //展示工作量时输出
    //cout<<"wirte unlock"<<endl;
}

// //输入排序， 写入缓存
// void SelectNode::write_rtt_notuse(vector<PAIR> &myvec){

//     struct_Node Beijing, Shanghai, Guangzhou, Changsha, Jinan;
//     //SelectNode anode;

//     if(!center_Information.empty()){
//             CenterInfo mycenter;
//             mycenter.deserialize(center_Information);    //b-1 s-2 g-3 c-4 j-5

    
//         // anode.mutex_lock(); cout<<"wirte lock"<<endl;
//         // anode.erase_v_delay();
//         mutex_lock(); //cout<<"wirte lock"<<endl; //展示工作量时输出
//         erase_v_delay();
//         for (int i = 0; i != myvec.size(); ++i) {
//             //展示工作量时输出
//             // cout << myvec[i].first << " --:-- " << myvec[i].second << endl;   //myvec[i].first是hostCenterName  myvec[i].seconds是延迟
//             if (myvec[i].first.compare(supercomputing_A) == 0){
//                 Beijing.location = mycenter.centerName["1"];   //TODO从客户端获取CenterName
//                 Beijing.ip_addr = mycenter.centerIP["1"];
//                 Beijing.port = mycenter.centerPort["1"];
//                 //anode.setNode_delay(Beijing);
//                 setNode_delay(Beijing);
//             }
//             else if (myvec[i].first.compare(supercomputing_B) == 0){
//                 Shanghai.location = mycenter.centerName["2"];
//                 Shanghai.ip_addr = mycenter.centerIP["2"];
//                 Shanghai.port = mycenter.centerPort["2"];
//                 //anode.setNode_delay(Shanghai);
//                 setNode_delay(Shanghai);
//             }
//             else if (myvec[i].first.compare(supercomputing_C) == 0){
//                 Guangzhou.location = mycenter.centerName["3"];
//                 Guangzhou.ip_addr = mycenter.centerIP["3"];
//                 Guangzhou.port = mycenter.centerPort["3"];
//                 //anode.setNode_delay(Guangzhou);
//                 setNode_delay(Guangzhou);
//             }
//             else if (myvec[i].first.compare(supercomputing_D) == 0){
//                 Changsha.location = mycenter.centerName["4"];
//                 Changsha.ip_addr = mycenter.centerIP["4"];
//                 Changsha.port =  mycenter.centerPort["4"];
//                 //anode.setNode_delay(Changsha);
//                 setNode_delay(Changsha);
//             }
//             else if (myvec[i].first.compare(supercomputing_E) == 0){
//                 Jinan.location = mycenter.centerName["5"];
//                 Jinan.ip_addr = mycenter.centerIP["5"];
//                 Jinan.port = mycenter.centerPort["5"];
//                 //anode.setNode_delay(Jinan);
//                 setNode_delay(Jinan);
//             }
//         }//for

//     }//if
//     else{
//         getCenterInformation();
//     }
//     //展示工作量时输出
//     //cout<<"wirte unlock"<<endl;
    
//     mutex_unlock();

// }


void SelectNode::getCenterInformation(){
    string ip = *(HvsContext::get_context()->_config->get<std::string>("default_ip"));
    string port = *(HvsContext::get_context()->_config->get<std::string>("default_port"));
    string url = "http://" + ip + ":" + port;   //http://localhost:9090

    string response = client->rpc->get_request(url, "/mconf/searchCenter");
    if((response == "fail") && center_Information.empty()){
        cout << "Not access to Server.." << endl;
    }
    else{
        center_Information = response;
    }
    
}


std::string SelectNode::getmapIdName(std::string centerName){
    //输入一个centername，返回一个centerid
    CenterInfo mycenter;
    mycenter.deserialize(center_Information); 
    vector<string>::iterator iter;
    for( iter = mycenter.centerID.begin(); iter!=mycenter.centerID.end(); iter++){
        if (mycenter.centerName[*iter] == centerName){
            return *iter;
        }
    }
    return "fail";
}


/*

SelectNode::SelectNode(){
        getCenterInformation();//获取center_Information

        if(!center_Information.empty()){
            CenterInfo mycenter;
            mycenter.deserialize(center_Information);    //b-1 s-2 g-3 c-4 j-5
            
            //读取配置文件，作为初值
            struct_Node local;
            vector<string>::iterator iter;
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

} 
*/

}//namespace hvs
