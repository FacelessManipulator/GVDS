/**
 * Created by Yaowen Xu on 2019-03-09.
 * 作者: Yaowen Xu
 * 时间: 2019-03-09
 * 工程: GVDS
 * 作者单位: 北京航空航天大学计算机学院-系统结构研究所
 */

#include "CmdLineProxy.h"

/*
 *  命令行组件添加子模块的时候，需要注册对应命令的命令行描述和处理函数的lamda表达式；
 */

CmdLineProxy::~CmdLineProxy() = default;

CmdLineProxy::CmdLineProxy(int argc, char **argv) {
    this->argc = argc;
    this->argv = argv;
    command = std::string(argv[0]); // 当前模块信息
    set_global_description(); // 设置全局配置信息
}

void CmdLineProxy::set_global_description() {
    cmdline_options = std::make_shared<po::options_description> ("命令格式: [COMMAND]  [OPTION] ... \n使用说明");
    //配置命令行信息描述模块
    std::string config_file;
    po::options_description generic("一般选项");
    generic.add_options()
            ("version,v", "输出可执行程序版本信息")
            ("help,h", "输出帮助信息");

    po::options_description advanced("高级选项");
    advanced.add_options()
            ("input-file,I", po::value< std::vector<std::string> >(), "输入文件")
            ("output-file,O", po::value< std::vector<std::string> >(), "输出文件");

    (*cmdline_options).add(generic).add(advanced);
}

void CmdLineProxy::set_command_description() {
    try{
        cmd_desc_func_map[command](cmdline_options); // 通过函数映射表，调用函数描述
    }catch (std::exception &e){
        cmdline_print("对不起，不支持当前命令！");
        exit(0);
    }
}

void CmdLineProxy::do_global_decode() {
    try {
        // 识别最后的没有特殊声明的文件， 全部解析为命令行输入文件；
        po::positional_options_description p;
        p.add("input-file", -1);

        sp_variables_map = std::make_shared<po::variables_map>();
        store(po::command_line_parser(this->argc, this->argv).
                options(*(this->cmdline_options)).positional(p).run(), *sp_variables_map);
        notify(*sp_variables_map);
        if (sp_variables_map->count("help")) {
            std::cout << *cmdline_options << std::endl;
            exit(0);
            return;
        }
        if (sp_variables_map->count("version")) {
            std::cout << "主程序版本: " << this->version << std::endl;
            exit(0);
            return;
        }

        // 显示输入文件列表，之后程序所有的接受的文件，都可以在vector<string> 之中进行接收到。
        if (sp_variables_map->count("input-file"))
        {
            //vm["input-file"].as< std::vector<std::string> >();
        }

        //通过当前子模块的名称，来调用每个模块的单独的函数
        do_module_router(); //根据模块名，来处理模块
    } catch (std::exception& e) {
        cmdline_print(e.what());
        cmdline_print("参数中出现不支持项目！请仔细阅读使用说明！");
        std::cout << *cmdline_options << std::endl;
        exit(-1); // 命令行参数中出现非法参数时，需要输出帮助，并停止执行;
    }
}

void CmdLineProxy::cmdline_print(const char * str) {
    // 命令行输出函数
    std::cerr << "命令行消息: " << str << std::endl;
}

void CmdLineProxy::do_module_router() {
    try{
        cmd_do_func_map[command](sp_variables_map); // 通过函数映射表, 调用当前模块处理函数；
    }catch (std::exception &e){
        cmdline_print(e.what()); // 输出当前异常
        cmdline_print("对不起，不支持当前命令！");
        exit(0);
    }
}

void CmdLineProxy::start() {
    set_command_description(); // 设置子模块配置信息
    do_global_decode(); // 做全局命令行解析,并执行子模块解析
}

template<class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
    copy(v.begin(), v.end(), std::ostream_iterator<T>(os, " "));
    return os;
}

void CmdLineProxy::test() {
    //设置当前命令行解析函数
    cmd_desc_func_map["command1"] =  [](std::shared_ptr<po::options_description> sp_cmdline_options)->void {
        po::options_description command1("子模块(1)命令");
        command1.add_options()
                ("name,n", po::value< std::vector<std::string>>(), "当前使用子模块的用户");
        sp_cmdline_options->add(command1); // 添加子模块命令行描述
    };
    //设置当前命令行处理函数
    cmd_do_func_map["command1"] =  [this](std::shared_ptr<po::variables_map> sp_variables_map)->void {
        if (sp_variables_map->count("name"))
        {
            // 示例代码，用来输出 name 参数队列！
            std::cout << (*sp_variables_map)["name"].as< std::vector<std::string>>() << std::endl;
        }
    };
}

void CmdLineProxy::print_options() {
    std::cout << *cmdline_options << std::endl;
}
