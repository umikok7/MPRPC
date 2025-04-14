#include "mprpcconfig.h"
#include <iostream>
#include <string>



//负责解析加载配置文件
void MprpcConfig::LoadConfigFile(const char* config_file){
    FILE *pf = fopen(config_file, "r");
    if(pf == nullptr){
        cout << config_file << " is not exist!" << endl;
        exit(EXIT_FAILURE);
    }
    // 1.注释 2.正确的配置项 3.去掉开头多余的空格
    // 逐行的处理配置文件内容
    while(!feof(pf)){
        char buf[512] = {0};
        fgets(buf, 512, pf);

        //去掉字符串前后多余空格
        string read_buf(buf); //转换为c++的字符串更利于处理
        Trim(read_buf);

        // 判断#注释
        if(read_buf[0] == '#' || read_buf.empty()){
            continue;
        }

        // 解析配置项
        int idx = read_buf.find('=');
        if(idx == -1){
            //配置项不合法
            continue;
        }

        string key, value;
        key = read_buf.substr(0, idx); 
        //去掉key前后的空格
        Trim(key);
        //例如rpcserverip = 127.0.0.1\n
        int endidx = read_buf.find('\n', idx);  //从idx的位置开始查找换行符
        value = read_buf.substr(idx + 1, endidx - idx - 1);
        //去掉value前后的空格
        Trim(value);

        m_configMap.insert({key, value});

    }
}



//根据key去查询配置项信息
string MprpcConfig::Load(const string &key){
    auto it = m_configMap.find(key);
    if(it == m_configMap.end()){
        return "";
    }
    return it->second;
}



//去除字符串前后的空格
void MprpcConfig::Trim(string &src_buf){
    //从前往后找 返回想要找的对应字符串下标
    int idx = src_buf.find_first_not_of(' '); 
    if(idx != -1){
        //说明字符串前面有空格
        // substr第一个是起始的下标 第二个是字符串的长度
        src_buf = src_buf.substr(idx, src_buf.size() - idx);
    }

    idx = src_buf.find_last_not_of(' ');
    if(idx != -1){
        //说明字符串后面有空格
        src_buf = src_buf.substr(0, idx + 1);
    }

}

