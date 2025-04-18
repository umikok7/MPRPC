#include "mprpcchannel.h"
#include "rpcheader.pb.h"
#include <sys/socket.h>
#include <sys/types.h>
#include "mprpcapplication.h"
#include <error.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "mprpccontroller.h"
#include "zookeeperutil.h"

/*
    数据格式：header_size + service_name method_name args_size + args
*/
//所有通过stub代理对象调用的rpc方法最终都会走到这里来，统一做rpc方法调用的数据序列化和网络发送
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
    google::protobuf::RpcController* controller, 
    const google::protobuf::Message* request,
    google::protobuf::Message* response, 
    google::protobuf::Closure* done){
        const google::protobuf::ServiceDescriptor* sd = method->service();
        std::string service_name = sd->name();
        std::string method_name = method->name();
        
        //获取参数的序列化字符串长度args_size
        int args_size = 0;
        std::string args_str;
        if(request->SerializeToString(&args_str)){
            args_size = args_str.size();
        }
        else{
            controller->SetFailed("serialise request error!");
            return;
        }

        // 定义rpc的请求header
        mprpc::RpcHeader rpcheader;
        rpcheader.set_service_name(service_name);
        rpcheader.set_method_name(method_name);
        rpcheader.set_arg_size(args_size);

        uint32_t header_size = 0;
        std::string rpc_header_str;
        if(rpcheader.SerializeToString(&rpc_header_str)){
            header_size = rpc_header_str.size();
        }
        else{
            controller->SetFailed("serialise rpc header error!");
            return;
        }

        // 接下来组织待发送的rpc请求的字符串
        std::string send_rpc_str;
        // 将header_size转化为4个字节存入send_rpc_str当中
        send_rpc_str.insert(0, std::string((char*)&header_size, 4));
        send_rpc_str += rpc_header_str; //rpcheader
        send_rpc_str += args_str; //args

        // 打印调试信息
        std::cout << " ===========" << std::endl;
        std::cout << "header_size:" << header_size << std::endl;
        std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
        std::cout << "service_name:" << service_name << std::endl;
        std::cout << "method_name:" << method_name << std::endl;
        std::cout << "args_str:" << args_str << std::endl;
        std::cout << " ===========" << std::endl;

        // 使用tcp编程，完成rpc方法的调用
        int clientfd = socket(AF_INET, SOCK_STREAM, 0);
        if(-1 == clientfd){
            char errtxt[512] = {0};
            sprintf(errtxt, "create socket error! errno:%d", errno);
            controller->SetFailed(errtxt);
            return;
        }


        // rpc调用方想调用service_name的method_name服务，需要查询zk上该服务所在的host信息
        zkClient zkCli;
        zkCli.start();
        std::string method_path = "/" + service_name + "/" + method_name;   //例如 /UserService/Login
        std::string host_data = zkCli.GetData(method_path.c_str());
        if(host_data == ""){
            controller->SetFailed(method_path + " is not exist!");
            return;
        }
        int idx = host_data.find(":");
        if(idx == -1){
            controller->SetFailed(method_path + " address is invalid!");
            return;
        }
        std::string ip = host_data.substr(0, idx);
        uint16_t port = atoi(host_data.substr(idx+1, host_data.size()-idx).c_str());


        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

        // 连接rpc服务节点
        if(-1 == connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr))){
            close(clientfd);
            char errtxt[512] = {0};
            sprintf(errtxt, "connect error! errno:%d", errno);
            controller->SetFailed(errtxt);
            return;
        }

        // 发送rpc请求
        if(-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0)){
            close(clientfd);
            char errtxt[512] = {0};
            sprintf(errtxt, "send error! errno:%d", errno);
            controller->SetFailed(errtxt);
            return;
        }

        // 接收rpc请求的响应值
        char recv_buf[1024] = {0};
        int recv_size = 0;
        if(-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0))){
            close(clientfd);
            char errtxt[512] = {0};
            sprintf(errtxt, "recv error! errno:%d", errno);
            controller->SetFailed(errtxt);
            return;
        }


        if(!response->ParseFromArray(recv_buf, recv_size)){
            close(clientfd);
            char errtxt[512] = {0};
            sprintf(errtxt, "parse error! response_str:%s", recv_buf);
            controller->SetFailed(errtxt);
            return;
        }

    }

    