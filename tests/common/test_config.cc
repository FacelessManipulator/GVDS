#include <iostream>
//#include <boost/program_options.hpp>
#include <libconfig.h++>
#include "config/ConfigureWrapper.h"
#include "config/SettingUtils.h"
#include "gtest/gtest.h"
#include <iomanip>

using namespace std;
using namespace libconfig;

/*
 *  配置文件主要有三个模块；
 *  1. ConfigureWrapper 模块：对 libconifg 模块进行了封装；
 *  2. SettingUtils 模块：实现了工具静态函数，方便解析 Setting 对象;
 *  3. libconfig 中 Setting 对象；
 *
 *  需要的 libconfig 库版本：1.7.2
 *  需要的 boost 库版本： 1.68.0
 */

//配置文件，测试输入文件。
#define CONFIGFILE "/tmp/hvs/tests/data/example.cfg"

//声明配置文件模块：
void search(); // 查找
void add(); // 增加
void del(); // 删除
void update(); // 修改
int simpleTest(); // 测试


TEST(ConfigTest, Simple) {
    EXPECT_TRUE(simpleTest()==0);
}

int simpleTest() {
    std::cout << "虚拟数据空间-配置模块" << std::endl;

    //配置文件查找样例：包括查找，增加，删除，修改 配置信息三个部分；
    search();
    add();
    del();
    update();
    //格式化文件
    cout << "== 格式化演示 ==" << endl;
    SettingUtils::format(CONFIGFILE, "/tmp/format.cfg");
    return 0;
}

void search(){
    ConfigureWrapper configureWrapper(CONFIGFILE);
    cout << "== 查找演示 ==" << endl;
    cout << "1. 通过路径查找" << endl;
    cout << SettingUtils::getStringValue(configureWrapper.lookUpByPath("."), "name") << endl;
    cout << SettingUtils::getIntValue(configureWrapper.lookUpByPath("inventory.books.[0]"), "title") << endl;
    cout << SettingUtils::getIntValue(configureWrapper.lookUpByPath("hours.mon"), "open") << endl;
    cout << SettingUtils::getIntValue(configureWrapper.lookUpByPath("hours.sun"), "open") << endl;
//    cout << endl;

    cout << "2. 通过下标查找" << endl;
    Setting& root = configureWrapper.getRoot();
    cout << SettingUtils::getStringValue(root, "name") << endl;
    cout << SettingUtils::getIntValue(root["inventory"]["books"][1], "title") << endl;
    cout << SettingUtils::getIntValue(root["hours"]["mon"], "close") << endl;
    cout << SettingUtils::getIntValue(root["hours"]["sun"], "close") << endl;
//    cout << endl;

    cout << "3. 展示所有电影" << endl;
    const Setting &movies = root["inventory"]["movies"];
    int count = movies.getLength();

    cout << setw(32) << left << "标题" << "  "
         << setw(12) << left << "格式" << "   "
         << setw(10) << left << "价格" << "  "
         << "数量"
         << endl;

    for(int i = 0; i < count; ++i)
    {
        const Setting &movie = movies[i];

        // Only output the record if all of the expected fields are present.
        string title, media;
        double price;
        int qty;

        if(!(movie.lookupValue("title", title)
             && movie.lookupValue("media", media)
             && movie.lookupValue("price", price)
             && movie.lookupValue("qty", qty)))
            continue;

        cout << setw(30) << left << title << "  "
             << setw(10) << left << media << "  "
             << '$' << setw(6) << right << price << "  "
             << qty
             << endl;
    }
    cout << endl;
}

void add(){ // 更新到 update.cfg 文件中
    ConfigureWrapper configureWrapper(CONFIGFILE);
    cout << "== 增加演示 ==" << endl;
    const char* outfile = "/tmp/add.cfg";
    Setting &root = configureWrapper.getRoot();

    if(! root.exists("inventory"))
        root.add("inventory", Setting::TypeGroup);

    Setting &inventory = root["inventory"];

    if(! inventory.exists("movies"))
        inventory.add("movies", Setting::TypeList);

    Setting &movies = inventory["movies"];

    // Create the new movie entry.
    Setting &movie = movies.add(Setting::TypeGroup);

    movie.add("title", Setting::TypeString) = "超人的大事记！";
    movie.add("media", Setting::TypeString) = "DVD";
    movie.add("price", Setting::TypeFloat) = 12.99;
    movie.add("qty", Setting::TypeInt) = 20;
    if(configureWrapper.writeFile(outfile)){
        cout << "文件成功更新到: " << outfile << " 文件中！" << endl;
    }
    cout << endl;
}

void del(){
    ConfigureWrapper configureWrapper(CONFIGFILE);
    cout << "== 删除演示 ==" << endl;
    const char* outfile = "/tmp/del.cfg";
    Setting &root = configureWrapper.getRoot();

    if(! root.exists("inventory"))
        root.add("inventory", Setting::TypeGroup);

    Setting &inventory = root["inventory"];

    if(inventory.exists("movies")){
        inventory.remove("movies");
    }

    if(configureWrapper.writeFile(outfile)){
        cout << "文件成功更新到: " << outfile << " 文件中！" << endl;
    }
    cout << endl;
}

void update(){
    ConfigureWrapper configureWrapper(CONFIGFILE);
    cout << "== 更新演示 ==" << endl;
    const char* outfile = "/tmp/update.cfg";
    Setting &root = configureWrapper.getRoot();

    if(! root.exists("inventory"))
        root.add("inventory", Setting::TypeGroup);

    Setting &inventory = root["inventory"];

    if(! inventory.exists("books"))
        inventory.add("books", Setting::TypeList);

    root ["name"] = "Xu Yao Wen";
    Setting &books = inventory["books"];
    books[0]["title"] =  "徐耀文";
    if(configureWrapper.writeFile(outfile)){
        cout << "文件成功更新到: " << outfile << " 文件中！" << endl;
    }
    cout << endl;
}