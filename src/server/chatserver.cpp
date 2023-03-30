#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"

#include <functional>
#include <string>

using namespace std;
using namespace placeholders;
using json = nlohmann::json;


//初始化对象 （构造函数）
Chatserver::Chatserver(EventLoop* loop,  //事件循环  epoll
            const InetAddress& listenAddr,// ip+port
            const string& nameArg)
        :_server(loop, listenAddr, nameArg)
{
    //注册 连接 回调
    _server.setConnectionCallback(std::bind(&Chatserver::onConnection, this, _1));

    //注册 消息 回调
    _server.setMessageCallback(std::bind(&Chatserver::onMessage, this, _1, _2, _3));

    //设置线程数量
    _server.setThreadNum(4);
}

//启动服务   开启事件循环   epoll_wait
void Chatserver::start(){
    _server.start();
}

//上报 用户的连接和断开 的回调函数   （epoll listenfd accept）
void Chatserver::onConnection(const TcpConnectionPtr& conn){
    if(!conn->connected())
    {
        //处理客户端异常退出
        //从map中删除这一对连接信息 并且修改db中用户状态
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }

}

//上报 用户的可读写事件 的回调函数
void Chatserver::onMessage(const TcpConnectionPtr& conn, //连接
                            Buffer* buffer,         //缓冲区
                            Timestamp time)
{
        string buf = buffer->retrieveAllAsString();
        //数据的反序列化
        json js = json::parse(buf);
        
        // 完全解耦 网络模块 和 业务模块 代码

        //通过js["msg_id"] 获取 相应的 业务 handler => 传入参数即可  conn js time

        //js["msgid"].get<int>() 将js中一个对象强转为int类型
        auto msgHandler = 
        ChatService::instance()->getHandler(js["msgid"].get<int>());
        //回调 该消息在chatservice中绑定好的 事件处理器， 来执行相应的操作
        msgHandler(conn, js, time);


    
}


