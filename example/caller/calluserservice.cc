#include <iostream>
#include "mprpcapplication.h"
#include "mprpcchannel.h"
#include "user.pb.h"

using namespace std;

int main(int argc, char **argv){
    // 整个程序启动后，想要使用mprpc框架来享用rpc服务，一定需要先调用框架的初始化函数（只初始化一次！）
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc方法Login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());


    // rpc方法的请求参数
    fixbug::LoginRequest request;
    request.set_name("zll");
    request.set_pwd("12345");
    // rpc方法的响应
    fixbug::LoginResponse response;
    // rpc方法的调用 同步rpc调用过程 MprpcChannel::callmethod
    stub.Login(nullptr, &request, &response, nullptr);

    if(0 == response.result().errcode()){
        cout << "rpc login response success:" << response.success() << endl;
    }
    else{
        cout << "rpc login response error:" << response.result().errmsg() << endl;
    }

    

    // 演示调用远程发布的rpc方法Register
    fixbug::RegisterRequest req;
    req.set_id(2000);
    req.set_name("mprpc");
    req.set_pwd("12345");
    fixbug::RegisterResponse rsp;

    // 以同步的方式发起rpc调用请求，等待返回结果
    stub.Register(nullptr, &req, &rsp, nullptr);
    if(0 == rsp.result().errcode()){
        cout << "rpc register response success:" << rsp.success() << endl;
    }
    else{
        cout << "rpc register response error:" << rsp.result().errmsg() << endl;
    }

    return 0;

}