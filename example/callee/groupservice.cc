#include "group.pb.h"
#include <iostream>
#include <unordered_map>
#include "mprpcapplication.h"
#include "rpcprovider.h"

using namespace std;
using namespace fixbug;


class GroupService : public fixbug::GroupServiceRpc{
public:
    // 本地业务
    bool createGroup(int userid, string groupname, int groupid, string groupdesc){
        cout << "do local service: createGroup!" << endl;
        cout << "userid = " << userid << " groupname = " << groupname << " groupid = " << groupid << " groupdesc = " << groupdesc;
        return true;
    }

    unordered_map <int, string> addGroup(int userid, string groupname){
        cout << "do local service: addGroup!" << endl;
        unordered_map<int, string> m1;
        m1.insert({userid, groupname});
        return m1;
    }

    // 重写基类的方法
    void createGroup(::google::protobuf::RpcController* controller,
        const ::fixbug::createGroupRequest* request,
        ::fixbug::createGroupResponse* response,
        ::google::protobuf::Closure* done){
            // 先获取做业务需要用到的参数
            int m_userid = request->userid();
            string m_groupname = request->groupname();
            int m_groupid = request->groupid();
            string m_groupdesc = request->groupdesc();
            // 执行本地业务
            bool creategroup_result = createGroup(m_userid, m_groupname, m_groupid, m_groupdesc);
            // 写响应
            response->mutable_result()->set_errmsg("");
            response->mutable_result()->set_errcode(0);
            response->set_success(creategroup_result);
            // 执行回调，目的是将响应对象的数据序列化和网络发送
            done->Run();
        }


    void addGroup(::google::protobuf::RpcController* controller,
        const ::fixbug::addGroupRequest* request,
        ::fixbug::addGroupResponse* response,
        ::google::protobuf::Closure* done){
            // 获取参数
            int m_id = request->userid();
            string g_name = request->groupname();
            // 执行本地业务
            unordered_map<int, string> m1 = addGroup(m_id, g_name);
            // 写响应
            response->mutable_result()->set_errcode(0);
            response->mutable_result()->set_errmsg("");
            for(auto & it : m1){
                string *group = response->add_group();
                *group = it.second;
            }
            // 执行回调，发送响应数据
            done->Run();

        }



private:

};

int main(int argc, char** argv){
    // 发布过程如下
    // 首先初始化框架
    MprpcApplication::Init(argc, argv);
    // 执行发布
    RpcProvider provider;
    provider.NotifyService(new GroupService());
    // 启动,阻塞等待有rpc调用
    provider.Run();

}
