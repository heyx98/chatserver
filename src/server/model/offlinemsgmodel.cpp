#include "offlinemsgmodel.hpp"
#include "db.hpp"

//存储用户的离线消息
void offLineMsgModel::insert(int userid, string msg)
{
    //1.组装sql语句             c_str() string->char*
    char sql[1024] = {0};
    sprintf(sql,"insert into offlinemessage values(%d, '%s')", userid, msg.c_str());
    
    MySQL mysql;
    if(mysql.connect()) //连接数据库
    { 
        mysql.update(sql);   //使用上述组装好的sql语句 更新数据库 插入一条数据 insert into
    }

}

//删除用户的离线消息
void offLineMsgModel::remove(int userid)
{
    //1.组装sql语句             c_str() string->char*
    char sql[1024] = {0};
    sprintf(sql,"delete from offlinemessage where userid=%d", userid);
    
    MySQL mysql;
    if(mysql.connect()) //连接数据库
    { 
        mysql.update(sql);   //使用上述组装好的sql语句 更新数据库 插入一条数据 insert into
    }

}

//查询用户的离线消息
vector<string> offLineMsgModel::query(int userid)
{
    //1.组装sql语句             c_str() string->char*
    char sql[1024] = {0};
    sprintf(sql,"select message from offlinemessage where userid = %d", userid);
    
    vector<string> vec;
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
                vec.push_back(row[0]);
            }
        }
        mysql_free_result(res);
        return vec;
    
    }
    return vec;

}