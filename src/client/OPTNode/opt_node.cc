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

vector<struct_Node> SelectNode::getNode(int choice){
    mutex_lock();
    if (choice == 1){
        return buf_delay;
    }
    else if (choice == 2){
        return buf_area;
    }
    else{
        return buf_delay;
    }
    mutex_unlock();
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
    create("opt_node");
}

void SelectNode::stop(){
    m_stop = true;
}

// void SelectNode::setOPT_init(){
//     getCenterInformation();//获取center_Information
//     std::cout << "center_Information : " << center_Information << std::endl;

//         if(center_Information!= "nothing"){
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
        cout << "write sleep 100" << endl;
        cout << "****"<<getCenterInfo() << endl;
        std::this_thread::sleep_for(std::chrono::seconds(100));
    }
}


//线程功能函数
int SelectNode::getRTT(map<string, double> &mymap){

    //string a = "www.baidu.com";
    if(center_Information!= "nothing"){
            CenterInfo mycenter;
            mycenter.deserialize(center_Information); 

        double rtt_beijing = subgetRTT(mycenter.centerIP["1"]);
        double rtt_shanghai = subgetRTT(mycenter.centerIP["2"]);
        double rtt_guangzhou = subgetRTT(mycenter.centerIP["3"]);
        double rtt_changsha = subgetRTT(mycenter.centerIP["4"]);
        double rtt_jinan = subgetRTT(mycenter.centerIP["5"]);

        mymap[supercomputing_A] = rtt_beijing;
        mymap[supercomputing_B] = rtt_shanghai;
        mymap[supercomputing_C] = rtt_guangzhou;
        mymap[supercomputing_D] = rtt_changsha;
        mymap[supercomputing_E] = rtt_jinan;

        return 0;
    }
    return -1;
}


double SelectNode::subgetRTT(string hostOrIp){
    
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
            cout << "PING " << hostOrIp << "(" << pingResult.ip.c_str() << "):" << pingResult.dataLen << " bytes data in ICMP packets." << endl;
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
        cout << nsend << "packets transmitted, " << nreceived << "received ,"<< ((nsend - nreceived) / nsend * 100) << "lost." << endl;
        rtt = rtt/forvalue * 1.0;
    }
    else{
        cout << "fail, rtt = 10000.0;" << endl;
        rtt = 10000.0;
    }
    return rtt;

}



void SelectNode::write_rtt(vector<PAIR> &myvec){

    struct_Node Beijing, Shanghai, Guangzhou, Changsha, Jinan;
    //SelectNode anode;

    if(center_Information!= "nothing"){
            CenterInfo mycenter;
            mycenter.deserialize(center_Information);    //b-1 s-2 g-3 c-4 j-5

    
    
        // anode.mutex_lock(); cout<<"wirte lock"<<endl;
        // anode.erase_v_delay();
        mutex_lock(); cout<<"wirte lock"<<endl;
        erase_v_delay();
        for (int i = 0; i != myvec.size(); ++i) {
            cout << myvec[i].first << " : " << myvec[i].second << endl;
            if (myvec[i].first.compare(supercomputing_A) == 0){
                Beijing.location = mycenter.centerName["1"];   //TODO从客户端获取CenterName
                Beijing.ip_addr = mycenter.centerIP["1"];
                Beijing.port = mycenter.centerPort["1"];
                //anode.setNode_delay(Beijing);
                setNode_delay(Beijing);
            }
            else if (myvec[i].first.compare(supercomputing_B) == 0){
                Shanghai.location = mycenter.centerName["2"];
                Shanghai.ip_addr = mycenter.centerIP["2"];
                Shanghai.port = mycenter.centerPort["2"];
                //anode.setNode_delay(Shanghai);
                setNode_delay(Shanghai);
            }
            else if (myvec[i].first.compare(supercomputing_C) == 0){
                Guangzhou.location = mycenter.centerName["3"];
                Guangzhou.ip_addr = mycenter.centerIP["3"];
                Guangzhou.port = mycenter.centerPort["3"];
                //anode.setNode_delay(Guangzhou);
                setNode_delay(Guangzhou);
            }
            else if (myvec[i].first.compare(supercomputing_D) == 0){
                Changsha.location = mycenter.centerName["4"];
                Changsha.ip_addr = mycenter.centerIP["4"];
                Changsha.port =  mycenter.centerPort["4"];
                //anode.setNode_delay(Changsha);
                setNode_delay(Changsha);
            }
            else if (myvec[i].first.compare(supercomputing_E) == 0){
                Jinan.location = mycenter.centerName["5"];
                Jinan.ip_addr = mycenter.centerIP["5"];
                Jinan.port = mycenter.centerPort["5"];
                //anode.setNode_delay(Jinan);
                setNode_delay(Jinan);
            }
        }//for

    }//if
    else{
        getCenterInformation();
    }
    //cout << "sleep 1" << endl;
    //sleep(1);
    cout<<"wirte unlock"<<endl;
    //anode.mutex_unlock();
    mutex_unlock();


}


void SelectNode::getCenterInformation(){
  cout << "enter: getCenterInformation" << endl;
  string response = client->rpc->get_request("http://localhost:9090", "/mconf/searchCenter");
    center_Information = response;
    cout << "end: getCenterInformation: " << response << endl;
}



/*

SelectNode::SelectNode(){
        getCenterInformation();//获取center_Information

        if(center_Information!= "nothing"){
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
