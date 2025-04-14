#pragma once

#include <unordered_map>
#include <string>
using namespace std;

//rpcserver_ip rpcserver_port zookeeper_ip zookeeper_port
//框架读取配置文件类
class MprpcConfig{
public:
    //负责解析加载配置文件
    void LoadConfigFile(const char* config_file);
    //查询配置项信息
    string Load(const string &key);
private:
    unordered_map<string, string> m_configMap;  //存储配置信息的键值对
    //去掉字符串前后的空格
    void Trim(string &src_buf);
};


