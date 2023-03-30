#include "usermodel.hpp"
#include "db.hpp"
#include <iostream>

using namespace std;

//user表的增加方法
bool UserModel::insert(User &user)
{
    //1.组装sql语句             c_str() string->char*
    char sql[1024] = {0};
    sprintf(sql,"insert into user(name, password, state) value('%s', '%s', '%s')", 
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());
    
    MySQL mysql;
    if(mysql.connect()){ //连接数据库
        if(mysql.update(sql)){ //使用上述组装好的sql语句 更新数据库 插入一条数据 insert into 

            //获取 插入成功 的用户数据 生成的主键id 并设置给user的id
            //mysql_insert_id() 返回给定的 connection 中上一步 INSERT 查询中产生的 AUTO_INCREMENT 的 ID 号
            user.setId(mysql_insert_id(mysql.getconnection()));
            return true;
        }
        return false; //表示注册失败
    }
    else
        return false;   

}

User UserModel::query(int id)
{
    //1.组装sql语句             c_str() string->char*
    char sql[1024] = {0};
    sprintf(sql,"select * from user where id = %d", id);
    
    MySQL mysql;
    if(mysql.connect()){ //连接数据库
        //使用上述组装好的sql语句 查询数据库 查询符合条件的一组数据 select * from user where

        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr){ //查询成功
            MYSQL_ROW row = mysql_fetch_row(res); //获取该id对应的一行数据 row
            if(row != nullptr){
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);

                mysql_free_result(res);
                return user;
            }
        }

        return User(); //表示查询失败 返回默认构造的User类对象    
    }

}

//更新用户的状态信息
bool UserModel::updateState(User user)
{   
    //1.组装sql语句             c_str() string->char*

    char sql[1024] = {0};
    sprintf(sql,"update user set state = '%s' where id = %d", user.getState().c_str(), user.getId());
    
    MySQL mysql;
    if(mysql.connect()){ //连接数据库
        if(mysql.update(sql)){ 

            //使用上述组装好的sql语句 更新数据库 更新一条数据 update . set . =  where . = 
            return true;
        }
        return false; //表示更新失败
    }
    else
        return false;

}

//重置用户的状态信息
void UserModel::resetState()
{
    char sql[1024] = "update user set state = 'offline' where state = 'online'";
    
    MySQL mysql;
    if(mysql.connect()){ //连接数据库

        //使用上述组装好的sql语句 更新数据库 更新一条数据 update . set . =  where . = 
        mysql.update(sql);        
    }

}