#include "friendmodel.hpp"
#include "db.hpp"

//添加好友关系
void FriendModel::insert(int userid, int friendid)
{
    //1.组装sql语句             c_str() string->char*
    char sql[1024] = {0};
    sprintf(sql,"insert into friend values(%d, %d)", userid, friendid);
    
    MySQL mysql;
    if(mysql.connect()) //连接数据库
    { 
        mysql.update(sql);   //使用上述组装好的sql语句 更新数据库 插入一条数据 insert into
    }

}

//返回用户好友列表
vector<User> FriendModel::query(int userid)
{
    //1.组装sql语句             c_str() string->char*
    char sql[1024] = {0};
    //多表联合查询  userid=>b  b.friendid => a.id =>a => a.id,a.name,a.state
    sprintf(sql,"select a.id,a.name,a.state from user a inner join friend b on b.friendid = a.id where b.userid = %d", userid);
    
    vector<User> vec;
    MySQL mysql;
    if(mysql.connect()){ //连接数据库
        //使用上述组装好的sql语句 查询数据库 查询符合条件的一组数据 select * from user where

        MYSQL_RES *res = mysql.query(sql);  //创建一个变量用于接收返回回来的数据
        if(res != nullptr)  //查询成功
        { 
            //把userid用户的所有离线消息放入vec中返回

            MYSQL_ROW row;      //获取该id对应的一行数据 row
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);

                vec.push_back(user);
            }
        }
        mysql_free_result(res);
        return vec;
    
    }
    return vec;

}