#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>
using namespace std;


MprpcConfig MprpcApplication::m_config;

void ShowArgsHelp(){
    cout << "format: command -i <configfile>" << endl;
}


void MprpcApplication::Init(int argc, char ** argv){
    if(argc < 2){
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }

    int c = 0;
    string config_file;
    while((c = getopt(argc, argv, "i:")) != -1){
        switch (c)
        {
        case 'i':
            config_file = optarg;  //例如用户输入./program -i config.conf 的时候接收到：config.conf
            break;
        case '?':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    //开始加载配置文件了 rpcserver_ip= rpcserver_port zookeeper_ip zookeeper_port=
    m_config.LoadConfigFile(config_file.c_str());

    //检验一下能否正确的读取配置文件
    // cout << "rpcserverip:" << m_config.Load("rpcserverip") << endl;
    // cout << "rpcserverport:" << m_config.Load("rpcserverport") << endl;
    // cout << "zookeeperip:" << m_config.Load("zookeeperip") << endl;
    // cout << "zookeeperport:" << m_config.Load("zookeeperport") << endl;

}



MprpcApplication& MprpcApplication::GetInstance(){
    static MprpcApplication app;
    return app;
}


MprpcConfig& MprpcApplication::GetConfig(){
    return m_config;
}
