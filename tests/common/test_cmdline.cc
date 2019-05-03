/**
 * Created by Yaowen Xu on 2019-03-09.
 * 作者: Yaowen Xu
 * 时间: 2019-03-09
 * 工程: HVSONE
 * 作者单位: 北京航空航天大学计算机学院-系统结构研究所
 */

#include "gtest/gtest.h"
#include "cmdline/CmdLineProxy.h"
#include <iostream>

using namespace std;

int cmdlineTest(){
    cout << endl;
//    const char* demo1[2] = {"command1", "-h"};
//    CmdLineProxy cmdLineProxy1(2, demo1);
//    cout << endl;
//    const char* demo2[2] = {"command1", "-v"};
//    CmdLineProxy cmdLineProxy2(2, demo2);
//    cout << endl;
    const char* demo3[7] = {"command1", "--name", "yaoxu", "--name", "superman", "--name", "password"};
//    const char* demo3[2] = {"command1", "-h"};
    CmdLineProxy cmdLineProxy1(7, demo3);
    cmdLineProxy1.test(); // 进行测试模块的功能, 非必须，展示 lamda 表达式使用；
    cmdLineProxy1.start(); // 开始解析函数
    return 0;
}

TEST(Cmdline_Test, Simple) {
    EXPECT_TRUE(cmdlineTest()==0);
}

