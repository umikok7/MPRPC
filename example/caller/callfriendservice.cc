#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"




int main(int argc, char **argv){
    // 整个程序启动后，想要使用mprpc框架来享用rpc服务，一定需要先调用框架的初始化函数（只初始化一次！）
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc方法Login
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());


    // rpc方法的请求参数
    fixbug::GetFriendListRequest request;
    request.set_userid(1000);
    // rpc方法的响应
    fixbug::GetFriendListResponse response;
    // rpc方法的调用 同步rpc调用过程 MprpcChannel::callmethod
    MprpcController controller;
    stub.GetFriendList(&controller, &request, &response, nullptr);


    // 一次性调用完成，读调用结果
    if(controller.Failed()){
        std::cout << controller.ErrorText() << std::endl;
    }
    else{
        if(0 == response.result().errcode()){
            cout << "rpc GetFriendList response success:" << endl;
            int size = response.friends_size();
            for(int i = 0; i < size; i++){
                cout << "index: " << i+1 << " name:" << response.friends(i) << endl;
            }
    
        }
        else{
            cout << "rpc GetFriendList response error:" << response.result().errmsg() << endl;
        }
    }


    return 0;

}

