#include "chatserver.hpp"
#include "chatservice.hpp"
#include <signal.h>

#include <iostream>
using namespace std;

//处理服务器 crtl + c 结束后 重置user的状态信息
void resethandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}
int main(int argc, char *argv[]){
     if(argc < 3) //命令输入有误  参数过少
    {
        cerr << " command invalid!  example: ./server 127.0.0.1 8000 " << endl;
        exit(-1);
    }
    //解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, resethandler);
    EventLoop loop;
    InetAddress addr(ip, port);

    Chatserver server(&loop, addr, "ChatServer");
    server.start();
    loop.loop();

    return 0;
}