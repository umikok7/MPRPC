#include <iostream>
#include <string>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "logger.h"
#include <vector>
using namespace fixbug;
using namespace std;


class FriendService : public fixbug::FriendServiceRpc {
public:
    std::vector<std::string> GetFriendsList(uint32_t userid){
        cout << "do GetFriendsList service! userid: " << userid << endl;
        vector<std::string> vec;
        vec.push_back("zll");
        vec.push_back("zlzz");
        vec.push_back("zlbb");
        return vec;
    }
    
    // 重写基类方法
    void GetFriendList(::google::protobuf::RpcController* controller,
                        const ::fixbug::GetFriendListRequest* request,
                        ::fixbug::GetFriendListResponse* response,
                        ::google::protobuf::Closure* done)
    {
        uint32_t userid = request->userid();

        std::vector<std::string> friendList = GetFriendsList(userid);  // 执行业务
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for(std::string &name : friendList){
            // 通过对response对象的内部friends数组中添加一个新的空字符串元素
            // 再通过指针来接收
            std::string *p = response->add_friends(); 
            // 通过指针间接修改friends中对字段
            *p = name;
        }

        //执行回调函数  执行响应对象数据的序列化和网络发送（都是框架完成的）
        done->Run();
        
    }

private:

};


int main(int argc, char** argv){
    // LOG_INFO("first log message!");
    // LOG_ERR("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);

    //调用框架的初始化操作
    MprpcApplication::Init(argc, argv);

    //provider是一个rpc网络服务对象。把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    //启动一个rpc服务发布节点   Run以后进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();


    return 0;
}

