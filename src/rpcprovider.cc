#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "zookeeperutil.h"
#include <string>
#include "rpcheader.pb.h"


/*
service_name => service描述
                service* 记录的服务对象
                method_name -> method对象方法
*/
// 这里是框架提供给外部使用的，可以发布rpc方法的函数接口 （感觉这个函数的功能就是用来识别哪个对象的哪个方法的）
void RpcProvider::NotifyService(google::protobuf::Service *service){
    ServiceInfo service_info;

    //获取了服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    //获取服务的名字
    string service_name = pserviceDesc->name();
    //获取服务的方法数量
    int methodCnt = pserviceDesc->method_count();

    // cout << "service_name:" << service_name << endl;
    LOG_INFO("service_name:%s", service_name.c_str());

    for(int i = 0; i < methodCnt; i++){
        //获取了服务对象指定下标的服务方法的描述（抽象描述）
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
        string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});
        LOG_INFO("method_name:%s", method_name.c_str());
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});

}


void RpcProvider::Run(){
    string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    // 创建tcpserver对象
    muduo::net::TcpServer server(&m_evLoop, address, "RpcProvider");

    // 绑定连接回调、消息读写回调方法
    server.setConnectionCallback(std::bind(&RpcProvider::onConnection, this, placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::onMessage, this, placeholders::_1, placeholders::_2, placeholders::_3));

    // 设置muduo库的线程数量
    server.setThreadNum(4);   

    // 把当前rpc节点上要发布的服务全部注册到zk上，让rpc client可以从zk上发现服务
    // session timeout 30s  zkclient的网络io线程 1/3 * timeout 的时间发ping消息（心跳）
    zkClient zkCli;
    zkCli.start();
    // 将service_name设置为永久性节点，method_name设置为临时性节点
    for(auto &sp : m_serviceMap){
        // /service_name
        std::string service_path = "/" + sp.first;
        zkCli.create(service_path.c_str(), nullptr, 0, 0);
        for(auto &mp : sp.second.m_methodMap){
            // /service_name/method_name
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port); // 存储当前这个rpc服务节点主机的ip和port
            zkCli.create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);  // ZOO_EPHEMERAL表示是一个临时性节点
        }
    }
 
    cout << "rpcprovider start service at ip:" << ip << " port:" << port << endl;

    // 启动网络服务
    server.start();
    m_evLoop.loop();
}


// 新的socket连接回调
void RpcProvider::onConnection(const muduo::net::TcpConnectionPtr &conn){
    if(!conn->connected()){
        // 和rpc client的连接断开
        conn->shutdown();
    }
}

/*
在框架内部 rpcProvider和rpcConsumer协商好之间通信用的protubuf数据类型
service_name method_name args 定义proto的message类型，进行数据的序列化和反序列化

header_size(4个字节) + header_str + args_str,
*/
// 读写已建立连接用户的读写事件回调 如果远程有一个rpc服务的调用请求，那么onmessage方法就会响应
void RpcProvider::onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buffer, muduo::Timestamp){
    // 网络上接收的远程rpc调用请求的字符流
    std::string recv_buf = buffer->retrieveAllAsString();
    // 从字符流中读取前4个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0);   //注意这个copy方法
    // 根据header_size读取数据头的原始字符流
    string rpc_header_str = recv_buf.substr(4, header_size);
    // 反序列化数据，得到rpc请求的详细信息
    mprpc::RpcHeader rpcHeader;
    string service_name;
    string method_name;
    uint32_t args_size;
    if(rpcHeader.ParseFromString(rpc_header_str)){
        // 数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.arg_size();
    }
    else{
        // 数据头反序列化失败
        cout << "rpc_header_str:" << rpc_header_str << " parse error!" << endl;
        return;
    }

    // 获取rpc方法参数的字符流数据
    string args_str = recv_buf.substr(4 + header_size, args_size);
    
    // 打印调试信息
    cout << " =======================" << endl;
    cout << "header_size:" << header_size << endl;
    cout << "rpc_header_str:" << rpc_header_str << endl;
    cout << "service_name:" << service_name << endl;
    cout << "method_name:" << method_name << endl;
    cout << "args_str:" << args_str << endl;
    cout << " ======================" << endl;


    // 获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if(it == m_serviceMap.end()){
        cout << service_name << " is not exist!" <<  endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if(it == m_serviceMap.end()){
        cout << service_name<< ":" << method_name << " is not exist!" <<  endl;
        return;
    }

    google::protobuf::Service *service = it->second.m_service; //获取service对象
    const google::protobuf::MethodDescriptor *method = mit->second;  //获取method方法

    // 生成rpc方法调用的请求request和响应response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New(); // 获取特定rpc方法的请求消息原型 通过方法描述符动态创建对应类型的请求对象
    if(!request->ParseFromString(args_str)){
        cout << "request parse error, content:" << args_str << endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();// 获取特定rpc方法的响应消息原型 通过方法描述符动态创建对应类型的请求对象

    // 给下面的method方法的调用，绑定一个Closure的回调函数，这个对象创建后被作为参数传入到service->CallMethod中
    // 当服务方法：例如FriendService::GetFriendList处理完了业务逻辑之后，调用 done->Run()就会触发绑定的sendRpcResponse方法
    // 这种基于回调的设计是高性能RPC框架的常见模式，它使服务实现能专注于业务逻辑，而将通信细节交给框架处理。
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider, 
                                                                    const muduo::net::TcpConnectionPtr&, 
                                                                    google::protobuf::Message*>(this, &RpcProvider::sendRpcResponse, conn, response);

    // 在框架上根据远端的rpc请求，调用当前rpc节点上发布的方法
    // 与new UserService().Login(controller, request, response, done)一一对应
    // 以friendservice来说，当程序执行到如下的时候会触发重写的GetFriendList方法，然后会在重写的方法内部调用业务逻辑
    // 也就是说，这个调用会根据method找到的去执行对应的实现方法
    // 接收请求 → 查找服务和方法 → service->CallMethod() → 重写的GetFriendList方法 → 业务逻辑GetFriendsList()
    service->CallMethod(method, nullptr, request, response, done);

}


// Closure的回调操作，用于序列化rpc的响应和网络发送
void RpcProvider::sendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response){
    string response_str;
    if(response->SerializeToString(&response_str)){
        // 序列化成功后，通过网络把rpc方法执行的结果发送回rpc调用方
        conn->send(response_str);
    }
    else{
        cout << "serialize response_str error!" << endl;
    }
    conn->shutdown(); //模拟http的短连接服务，由rpcprovider主动断开连接
}

