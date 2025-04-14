#include "test.pb.h"
#include <iostream>
#include <string>
using namespace std;
using namespace fixbug;

int main(){
    // LoginResponse rsp;
    // RequestCode* rc = rsp.mutable_result();
    // rc->set_errcode(1);
    // rc->set_errmsg("登陆处理失败了");

    GetFriendListResponse rsp;
    RequestCode* rc = rsp.mutable_result();   //mutable_ 获取类中的类的对象
    rc->set_errcode(0);
    User *user1 = rsp.add_friend_list();       //add_   获取列表的对象
    user1->set_age(20);
    user1->set_name("zll");
    user1->set_sex(User::WOMAN);

    User *user2 = rsp.add_friend_list();    //add_
    user2->set_age(20);
    user2->set_name("zlbb");
    user2->set_sex(User::WOMAN);
    cout << rsp.friend_list_size() << endl;

    return 0;
}


int main1(){
    //  封装了login请求对象的数据
    LoginRequest req;
    req.set_name("zll");
    req.set_pwd("123456");

    //对象数据序列化
    string send_str;
    if(req.SerializeToString(&send_str)){
        cout << send_str << endl;
    }

    //从send_str反序列化为一个login请求对象
    LoginRequest reqB;
    if(reqB.ParseFromString(send_str)){
        cout << reqB.name() << endl;
        cout << reqB.pwd() << endl;
    }

    return 0;
}

