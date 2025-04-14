#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <iostream>



void global_watcher(zhandle_t *zh, int type, 
    int state, const char *path,void *watcherCtx){
        // 回调的消息类型是与会话相关的消息类型
        if(type == ZOO_SESSION_EVENT){
            // zkclient与zkserver连接成功
            if(state == ZOO_CONNECTED_STATE){
                // sem_t *sem = (sem_t*)zoo_get_context(zh);
                zkClient *cli = (zkClient*)zoo_get_context(zh);
                //执行到此处后说明接收到了ZOO_CONNECTED_STATE的状态了，直接唤醒信号量停止sem_wait的阻塞
                //至于为什么要接收到ZOO_CONNECTED_STATE才算是任务被建立直接跳转看zookeeper_init的官方描述
                // sem_post(sem); 
                std::unique_lock<std::mutex> locker(cli->m_mutex);
                cli->m_connected = true;
                cli->m_cond.notify_one();
            }
        }
    }


zkClient::zkClient() : m_zhandle(nullptr), m_connected(false){

}


zkClient::~zkClient(){
    if(m_zhandle != nullptr){
        zookeeper_close(m_zhandle);  // 关闭句柄释放资源
    }
}



// zkclient启动连接zkserver
void zkClient::start(){
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;

    /*
    zookeeper_mt:多线程版本
    zookeeper的API客户端程序提供了三个线程
    API调用线程
    网络的IO线程 pthread_create poll
    watcher回调线程
    */
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if(m_zhandle == nullptr){
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // sem_t sem;
    // sem_init(&sem, 0, 0);
    // zoo_set_context(m_zhandle, &sem);

    // sem_wait(&sem);

    // 传递this指针作为上下文
    zoo_set_context(m_zhandle, this);

    std::unique_lock<mutex> locker(m_mutex);
    // 避免虚假唤醒
    while(!m_connected){
        m_cond.wait(locker);
    }

    std::cout << "zookeeper_init success!" << std::endl;
}


// 在zkserver上根据指定的path创建znode节点
void zkClient::create(const char *path, const char *data, int datalen, int state){
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
    // 先判断path表示的znode节点是否存在，如果存在就不重复创建了
    flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if(ZNONODE == flag){
        // 说明path的node节点不存在，那么就创建指定path的znode节点
        flag = zoo_create(m_zhandle, path, data, datalen, 
               &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
        if(flag == ZOK){
            std::cout << "znode create success... path:" << path << std::endl;
        }
        else{
            std::cout << "flag: " << flag << std::endl;
            std::cout << "znode create error... path:" << path << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}


// 根据参数指定的znode节点路径，或者znode节点的值
std::string zkClient::GetData(const char *path){
    char buffer[128];
    int bufferlen = sizeof(buffer);
    int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
    if(flag != ZOK){
        std::cout << "get znode error... path:" << path << std::endl;
        return "";
    }else{
        return buffer;
    }
}

