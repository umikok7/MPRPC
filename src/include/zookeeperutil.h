#pragma once

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>
#include <condition_variable>

// 封装的zk客户端类
class zkClient{
public:
    zkClient();
    ~zkClient();
    // zkclient启动连接zkserver
    void start();
    // 在zkserver上根据指定的path创建znode节点
    void create(const char *path, const char *data, int datalen, int state);
    // 根据参数指定的znode节点路径，或者znode节点的值
    std::string GetData(const char *path);
private:
    // zk的客户端句柄
    zhandle_t *m_zhandle;

    // 用于同步的互斥锁和条件变量
    std::mutex m_mutex;
    std::condition_variable m_cond;
    bool m_connected;

    // 声明全局回调函数为友元
    friend void global_watcher(zhandle_t *zh, int type, 
        int state, const char *path,void *watcherCtx);

};