#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <vector>


using namespace muduo;
using namespace std;

//获取单例 ChatService 对象的接口函数
ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}

//注册消息以及对应的 回调 操作      绑定器 std::bind  回调 
ChatService::ChatService()
{
    //  unordered_map<int, MsgHandler> _msgHandlerMap;
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});

    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    //连接redis服务器
    if(_redis.connect())
    {
        //设置上报消息回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubsrcibeMessage, this, _1, _2));
        
    }
}

//获取消息对应的处理器 handler
MsgHandler ChatService::getHandler(int msgid)
{
    //记录错误日志 msgid没有对应的事件处理回调
    auto it = _msgHandlerMap.find(msgid);

    //没有找到
    if(it == _msgHandlerMap.end()) 
    {
        //返回一个默认的handler 空操作
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time){
            //用LOG_ERROR打印日志不要加endl
            LOG_ERROR << "msgid : " << msgid << " can not find handler!"; 
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}

 //处理登陆业务   id password
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time )
{
    //LOG_INFO <<"do login service!";
    int id = js["id"].get<int>();
    string pwd = js["password"];

    User user = _usermodel.query(id);

    if(user.getId() == id && user.getPwd() == pwd){
        // id 存在且pwd正确  
        if(user.getState() == "online")
        {
            //该用户已经登陆 不允许再次登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "该账号已经登录，请重新输入账号";
            conn->send(response.dump());
        }
        else
        {
            //登录成功  记录用户连接信息
            {
                //程序在std::lock_guard生命周期内加锁和解锁，其中加锁和解锁分别在构造函数和析构函数中完成
                //在定义lock_guard的地方会调用构造函数加锁，在离开定义域的话lock_guard就会被销毁，调用析构函数解锁
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id,conn});
            }
            //id用户登录成功后 向redis订阅channel(id)
            _redis.subscribe(id);
            
            //登陆成功 更新用户状态信息 state: offline->online
            user.setState("online");
            _usermodel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["id"] = user.getId();
            response["errno"] = 0;
            response["name"] = user.getName();

            //查询该用户是否有离线信息
            vector<string> vec = _offlineMsgModel.query(id);
            if(!vec.empty())
            {
                response["offlinemsg"] = vec;
                //读取用户离线消息后 将该离线消息删除
                _offlineMsgModel.remove(id);
            }

            //查询该用户的好友信息并返回
            vector<User> userVec = _friendModel.query(id);
            if(!userVec.empty())
            {
                vector<string> vec2;
                for(User user : userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();

                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            /*
            将JSON对象序列化为字符串格式。
            nlohmann::json类的dump()函数可以帮助我们将JSON对象序列化为字符串形式，以便于存储和传输。
            */
            conn->send(response.dump());
        }
    }

    else
    {
        //该用户不存在  或者用户存在password错误  登陆失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户名或密码错误";
        conn->send(response.dump());
    }

}

//处理注册业务  name password
void ChatService::reg (const TcpConnectionPtr &conn, json &js, Timestamp time )
{
    //LOG_INFO << "do reg service!";
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);

    bool state = _usermodel.insert(user);
    if(state){
        //注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        /*
        将JSON对象序列化为字符串格式。
        nlohmann::json类的dump()函数可以帮助我们将JSON对象序列化为字符串形式，以便于存储和传输。
        */
        conn->send(response.dump());
    }
    
    else{
        //注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = 1;

        conn->send(response.dump());
    }
}
//处理注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if(it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }

    //用户注销 就是下线 在redis中取消订阅channel
    _redis.unsubscribe(userid);

    //更新DB中用户的状态信息

    User user(userid, "", "", "offline");
    _usermodel.updateState(user);

}
//处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    //由于可提供参数只有conn 要通过该conn 在 _userConnMap中遍历找到对应的id 
    //从map中删除这一对连接信息 并且修改db中用户状态
    {
        lock_guard<mutex> lock(_connMutex);
        for(auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it){
            if(it->second == conn)
            {
                ////删除连接信息
                user.setId(it->first);
                _userConnMap.erase(it);
                break; //找到了就结束循环
            }
        }
    }

    //客户端异常退出 就是下线 在redis中取消订阅channel
    _redis.unsubscribe(user.getId());

    //更新DB中用户的状态信息
    if(user.getId() != -1)
    {
        user.setState("offline");
        _usermodel.updateState(user);
    }
    
     
}

//一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["to"].get<int>();

    //访问线程共享的数据_userConnMap  加锁
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if(it != _userConnMap.end()) //找到了
        {
            //toid用户在线 直接转发消息 服务器主动send数据给toid用户
            it->second->send(js.dump());
            return;
        }
    }

    //查询toid是否在线
    User user = _usermodel.query(toid);
    if(user.getState() == "online")
    {
        _redis.publish(toid, js.dump());
        return;
    }

    // toid用户不在线 存储离线消息
    _offlineMsgModel.insert(toid, js.dump());

}

//添加好友业务
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int frienfid = js["friendid"].get<int>();

    //存储好友信息
    _friendModel.insert(userid, frienfid);
}

//创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    //存储新创建的群组信息
    Group group(-1, name, desc);
    if(_groupModel.creatGroup(group))
    {
        //存储群组创建人的信息
        _groupModel.addGroup(userid, group.getId(), "creator");
    }


}

    //加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");

}

//群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);
    lock_guard<mutex> lock(_connMutex);
    for(int id : useridVec)
    {
        auto it = _userConnMap.find(id);
        if(it != _userConnMap.end()) //找到了
        {
            //转发群消息
            it->second->send(js.dump());
        }
        else
        {
            //查询id是否在线
            User user = _usermodel.query(id);
            if(user.getState() == "online")
            {
                _redis.publish(id, js.dump());
                return;
            }
             //存储离线群消息
            _offlineMsgModel.insert(id, js.dump());
        }
    }
}

//从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubsrcibeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if(it != _userConnMap.end()) //找到了
    {
        //toid用户在线 直接转发消息 服务器主动send数据给toid用户
        it->second->send(msg);
        return;
    }
    //存储离线群消息
    _offlineMsgModel.insert(userid, msg);

}

//服务器异常 业务重置方法
void ChatService::reset()
{
    //把online用户的状态 重置为 ofline
    _usermodel.resetState();
}