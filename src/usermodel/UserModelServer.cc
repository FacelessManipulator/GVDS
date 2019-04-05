/*
Author:Liubingqi
date:2019.03.21

g++ -c UserModelServer.cc
g++ -c hvsrest.cc -lpistache  -std=c++11
g++ -o user UserModelServer.o hvsrest.o -lpistache -std=c++11

./user 5
*/


#include <iostream>
#include "common/JsonSerializer.h"
#include "context.h"

#include "datastore/datastore.h"

#include "usermodel/UserModelServer.h"
#include "MD5.h"


using namespace std;


//using namespace hvs;
//object 



namespace hvs{
UserModelServer* UserModelServer::instance = nullptr;


//账户注册
void UserModelServer::UserRegisterRest(const Rest::Request& request, Http::ResponseWriter response){
    cout << "====== start UserModelServer function: UserRegisterRest ======"<< endl;

    auto info = request.body();
    cout << info << endl;
/*
    Account person("lbq", "123456", "456", "XXXXXX@163.com", "15012349876", "xueyuanlu",  "Beihang", "has");
    person.sc.location_scacc["beijing"] = "local_lbq";
    person.sc.scacc_password["local_lbq"] = "654321";
    person.sc.location_scacc["shanghai"] = "local_lbq1";
    person.sc.scacc_password["local_lbq1"] = "654321";
*/


    Account person;
    person.deserialize(info);  

    string result = UserRegister(person);

    cout<<"result:"<<result<<endl;
    response.send(Http::Code::Ok, result); //point
    cout << "====== end UserModelServer function: UserRegisterRest ======"<< endl;
}


string UserModelServer::UserRegister(Account &person){
    cout << "enter UserRegister ======"<< endl;

    std::string person_key = person.accountID;
    //检查是否存在key，不存在，则进行下面代码开始注册，存在则返回注册失败
    std::shared_ptr<hvs::CouchbaseDatastore> f1_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("account_map_id"));
    f1_dbPtr->init();
    auto [pvalue, error_0] = f1_dbPtr->get(person.accountName);
    if(error_0 == 0){  
        return "Account name already exists!";
    }


    //不存在此key，开始注册
    //写入account_map_info表
    AccountPair acc_pair(person.accountName, person.accountID);
    string pari_key = person.accountName;
    string pair_value = acc_pair.serialize();
    
    int f1_flag = f1_dbPtr->set(pari_key, pair_value);
    if (f1_flag != 0){
        string result_0 = "Registration fail: DB[account_map_id] write fail;";
        return result_0;
    }


    //写入account_info表
    std::string person_value = person.serialize();  //json

    std::cout<<person_key<<endl;
    std::cout<<person_value<<endl;
   
    std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("account_info"));
    f0_dbPtr->init();
    
    int flag = f0_dbPtr->set(person_key, person_value);
    if (flag != 0){
        string result_0 = "Registration fail: DB[account_info] write fail;";
        return result_0;
    }
    else{
        string result_1 = "Dear:" + person.accountName + ", registration success";
        return result_1;
    }    
    //string result = "Dear user: " + person.HVSAccountName + ", registration success. Please log in with your username and password";
    //return result;
}



//账户登录
void UserModelServer::UserLoginRest(const Rest::Request& request, Http::ResponseWriter response){
    cout << "====== start UserModelServer function: UserLoginRest ======"<< endl;
    //=====
    printCookies(request);
    //=====
    auto info = request.body(); 
    cout << info << endl;   //账户名，密码
    
    //解析账户名密码
    AccountPass acc_pass;
    acc_pass.deserialize(info);

    bool is_success = UserLogin(acc_pass.accountName, acc_pass.Password);

    if (is_success){
        //md5
        string origin_str = acc_pass.accountName + acc_pass.Password; //再加上url以及时间等等信息
        string mtoken = md5(origin_str);
        std::shared_ptr<hvs::CouchbaseDatastore> f2_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
            hvs::CouchbaseDatastore("token_info"));
        f2_dbPtr->init();
        string value ="1";
        int login_flag = f2_dbPtr->set(mtoken, value);
        if (login_flag != 0){
            response.send(Http::Code::Ok, "login fail!,token set fail");
        }
        else{
            response.cookies().add(Http::Cookie("token", mtoken));
            response.send(Http::Code::Ok, "login success!"); //point
        }
    }
    else{
        response.send(Http::Code::Ok, "login fail!");
    }

    //auto pmtoken = response.headers().get("Token");
    //cout << "pmtoken: " << pmtoken <<endl;
    //cout << "*pmtoken: " << *pmtoken <<endl;
   

    cout<<"====== end UserModelServer function: UserLoginRest ======"<<endl;    
}

bool UserModelServer::UserLogin(std::string account, std::string pass){
    cout << "enter UserLogin"<< endl;
    //AccountPair中实现新类，只存账户名，和id，这两个信息
    //查询账户名对应的id，作为数据库查询的key

    std::shared_ptr<hvs::CouchbaseDatastore> f1_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("account_map_id"));

    f1_dbPtr->init();

    //获取account对应的id    [若数据库没有此key，则返回登录失败的代码]
    auto [pvalue, error_0] = f1_dbPtr->get(account);
    if(error_0){
        cout << "DB[account_map_id]: No such account" <<endl;
        return false;
    }
    AccountPair login_acc_pair;
    login_acc_pair.deserialize(*pvalue);

    //使用获取的id查询数据库中密码，并比较
    string key = login_acc_pair.accountID;
    //string path = "HVSPassword";
    std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("account_info"));
    
    f0_dbPtr->init();
    //[若数据库没有此key，则返回登录失败的代码]
    //auto [pPass, error_1] = f0_dbPtr->get(key, path);   *pPass输出带引号
//tmp
    auto [pvalue_2, error_2] = f0_dbPtr->get(key);
    if(error_2){
        cout << "DB[account_info]: No such account" <<endl;
        return false;
    }

    Account tmp;
    tmp.deserialize(*pvalue_2);

    //pass == *pPass 密码一致
    if (!pass.compare(tmp.Password)) {
        string result = "login success";
        return true;
    } 

    else{
        cout<<"pass = " << pass <<endl;
        cout <<"tmp.Password = " << tmp.Password << endl;
        string result = "login fail";
        return false;
    }

}


//账户信息查询
void UserModelServer::getUserinfoRest(const Rest::Request& request, Http::ResponseWriter response){
    cout << "====== start UserModelServer function: getUserinfoRest ======"<< endl;

    bool valid = auth_token(request);
    if (!valid){
        response.send(Http::Code::Unauthorized, "Verification failed, access denied");
        return;
    }
    
    
    auto uuid = request.param(":name").as<std::string>();
    
    bool is_get_success = true;
    // include your functin
    string data = getUserinfo(uuid, is_get_success);
    if(is_get_success){
        response.send(Http::Code::Ok, data);
    }
    else{
        response.send(Http::Code::Not_Found, data); //point
    }
    
    cout << "====== end UserModelServer function: getUserinfoRest ======"<< endl;
}

string UserModelServer::getUserinfo(string uuid, bool &is_get_success){
    cout << "enter getUserinfo"<< endl;
/*
    map<string, string> usermap;

    usermap["username"] = uuid;
    usermap.insert(pair<string, string>("email", "liubingqi112@163.com"));

    string nameValue = usermap["username"];
    string emailValue = usermap["email"];
    //for (map<string, string>::const_iterator iter = usermap.begin( ); iter != usermap.end( ); ++iter)


    string data = "{\"name\":\"" + nameValue + "\", \"email\":\"" + emailValue + "\"}" ;
    return data;

*/
    string key = uuid;

    std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("account_info"));
    f0_dbPtr->init();

    //判断key是否存在
    auto [pvalue, error] = f0_dbPtr->get(key);
    if (error){
        is_get_success = false;
        return "Search fail, try again.";
    }

    cout<<"pvalue:"<< *pvalue <<endl;
    return *pvalue;

}



void UserModelServer::modifyUserinfoRest(const Rest::Request& request, Http::ResponseWriter response){
    cout << "====== start UserModelServer function: modifyUserinfoRest ======"<< endl;

    bool valid = auth_token(request);
    if (!valid){
        response.send(Http::Code::Unauthorized, "Verification failed, access denied");
        return;
    }

    auto info = request.body();
    cout << info << endl;

    Account person;
    person.deserialize(info);  

    string result = modifyUserinfo(person);

    cout<<"result:"<<result<<endl;
    response.send(Http::Code::Ok, result); //point

    cout << "====== end UserModelServer function: modifyUserinfoRest ======"<< endl;
}

string UserModelServer::modifyUserinfo(Account &person){

    //更新account_info表
    string person_key = person.accountID;
    string person_value = person.serialize();  //json

    cout << person_key << endl;
    cout << person_value << endl;
   
    std::shared_ptr<hvs::CouchbaseDatastore> f0_dbPtr = std::make_shared<hvs::CouchbaseDatastore>(
        hvs::CouchbaseDatastore("account_info")); 
    f0_dbPtr->init();
    
    int flag = f0_dbPtr->set(person_key, person_value);
    if (flag != 0){
        return "Modify fail: DB[account_info] update fail";
    }
    else{
        return "Modify success";
    }    

}





//MD5

string md5(string strPlain)
{
		MD5_CTX mdContext;
		int bytes;
		unsigned char data[1024];
 
		MD5Init(&mdContext);
		MD5Update(&mdContext, (unsigned char*)const_cast<char*>(strPlain.c_str()), strPlain.size());
		MD5Final(&mdContext);
 
		string md5;
		char buf[3];
		for (int i = 0; i < 16; i++)
		{
			sprintf(buf, "%02x", mdContext.digest[i]);
			md5.append(buf);
		}
		return md5;
}

}// namespace hvs