#include "mprpcapplication.h"
#include <iostream>
#include "group.pb.h"

using namespace std;

int main(int argc, char** argv){
    // 使用rpc框架流程如下
    // 首先启用框架
    MprpcApplication::Init(argc, argv);
    // 通过槽函数调用
    fixbug::GroupServiceRpc_Stub stub(new MprpcChannel());
    fixbug::createGroupRequest createReq;
    // 设置请求参数
    createReq.set_userid(1); 
    createReq.set_groupid(1);
    createReq.set_groupname("happy family");
    createReq.set_groupdesc("use for chat");
    // rpc方法的响应
    fixbug::createGroupResponse createRsp;
    // 通过框架调用方法实现业务,获取响应
    MprpcController createCtl;
    stub.createGroup(&createCtl, &createReq, &createRsp, nullptr);
    if(createCtl.Failed()){
        cout << createCtl.ErrorText() << endl;
    }
    else{
        if(0 == createRsp.result().errcode()){
            cout << "rpc createGroup response success!" << endl;
        }
        else{
            cout << "rpc createGroup response error: " << createRsp.result().errmsg() << endl;
        }
    }

    // 同样的调用一下addgroup方法
    fixbug::addGroupRequest addReq;
    addReq.set_userid(1);
    addReq.set_groupname("happy family");

    fixbug::addGroupResponse addRsp;
    MprpcController addCtl;
    stub.addGroup(&addCtl, &addReq, &addRsp, nullptr);

    if(addCtl.Failed()){
        cout << addCtl.ErrorText() << endl;
    }
    else{
        if(0 == addRsp.result().errcode()){
            cout << "rec addGroup response success!" << endl;
            int size = addRsp.group_size();
            for(int i = 0; i < size; i++){
                cout << "index = " << i+1 << " group name = " << addRsp.group(i) << endl;
            }
        }
        else{
            cout << "rpc addGroup response error: " << addRsp.result().errmsg() << endl;
        }
    }

    
}

