#ifndef CHATSERVER_H
#define CHATSERVER_H
/*
muduo网络库提供了两个主要的类
TcpServer : 用于编写服务器程序的
TcpClient : 用于编写客户端程序

底层实现： epoll + 线程池
能够把网络I/O代码和业务代码分开
                用户的连接和断开   用户的可读写事件 
*/

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;


//基于muduo网络库开发服务器程序
/*
1.组合TcpServer对象
2.创建Eventloop事件循环对象的指针
3.明确TcpServer构造函数需要什么参数 ， 编写Chatserver的构造函数
    3.1 给服务器注册 用户的连接和断开 回调
    3.2 给服务器注册 用户的可读写事件 回调
    3.3 设置服务器端合适的线程数量
*/

class Chatserver{
public:
    //初始化对象 （构造函数）
    Chatserver(EventLoop* loop,  //事件循环  epoll
            const InetAddress& listenAddr,// ip+port
            const string& nameArg);      //服务器的名字

    //启动服务   开启事件循环   epoll_wait
    void start();

private:
    //上报 用户的连接和断开 的回调函数   （epoll listenfd accept）
    void onConnection (const TcpConnectionPtr& conn);
    
    //上报 用户的可读写事件 的回调函数
    void onMessage (const TcpConnectionPtr& conn, //连接
                            Buffer* buffer,         //缓冲区
                            Timestamp time);      //接收到数据的时间信息
    TcpServer _server;  // 1 实现服务器功能的类对象
    EventLoop *_loop;   //2 epoll 指向事件循环对象的指针
};


#endif