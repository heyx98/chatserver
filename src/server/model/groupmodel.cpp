#include "groupmodel.hpp"
#include "db.hpp"

//创建群组
bool GroupModel::creatGroup(Group &group)
{
    //1.组装sql语句             c_str() string->char*
    char sql[1024] = {0};
    sprintf(sql,"insert into allgroup(groupname, groupdesc) value('%s', '%s')", 
            group.getName().c_str(), group.getDesc().c_str());
    
    MySQL mysql;
    if(mysql.connect()){ //连接数据库
        if(mysql.update(sql)){ //使用上述组装好的sql语句 更新数据库 插入一条数据 insert into 

            //获取 插入成功 的用户数据 生成的主键id 并设置给user的id
            //mysql_insert_id() 返回给定的 connection 中上一步 INSERT 查询中产生的 AUTO_INCREMENT 的 ID 号
            group.setId(mysql_insert_id(mysql.getconnection()));
            return true;
        }
        return false; //表示注册失败
    }
    else
        return false;

}

//加入群组
void GroupModel::addGroup(int userid, int groupid, string role)
{
    //1.组装sql语句             c_str() string->char*
    char sql[1024] = {0};
    sprintf(sql,"insert into groupuser values(%d, %d, '%s')", groupid, userid, role.c_str());
    
    MySQL mysql;
    if(mysql.connect()) //连接数据库
    { 
        mysql.update(sql);   //使用上述组装好的sql语句 更新数据库 插入一条数据 insert into
    }

}

//查询用户所在群组信息
vector<Group> GroupModel::queryGroup(int userid)
{
    //1.组装sql语句             c_str() string->char*
    char sql[1024] = {0};
    //多表联合查询  
    sprintf(sql,"select a.id,a.groupname,a.groupdesc from allgroup a inner join \
    groupuser b on a.id = b.groupid where b.userid = %d", userid);
    
    vector<Group> groupVec;
    MySQL mysql;
    if(mysql.connect())
    { //连接数据库
        //使用上述组装好的sql语句 查询数据库 查询符合条件的一组数据 select * from user where

        MYSQL_RES *res = mysql.query(sql);  //创建一个变量用于接收返回回来的数据
        if(res != nullptr)  //查询成功
        { 
            MYSQL_ROW row;      //获取该id对应的一行数据 row

            //查出userid所有的群组信息
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);

                groupVec.push_back(group);
            }
        }
        mysql_free_result(res);

        //查询群组的用户信息  vector<GroupUser> users; 
        for(Group &group : groupVec)
        {
            //1.组装sql语句             c_str() string->char*
            char sql[1024] = {0};
            sprintf(sql,"select a.id,a.name,a.state,b.grouprole from user a inner join \
                    groupuser b on b.userid = a.id where b.groupid = %d", 
                    group.getId());
            
            
            MYSQL_RES *res = mysql.query(sql);
            if(res != nullptr)
            { //查询成功
                MYSQL_ROW row = mysql_fetch_row(res); //获取该id对应的一行数据 row
                if(row != nullptr)
                {
                    GroupUser user;
                    user.setId(atoi(row[0]));
                    user.setName(row[1]);
                    user.setState(row[2]);
                    user.setRole(row[3]);

                    group.getUsers().push_back(user);
                }
                mysql_free_result(res);
            }
        }

    return groupVec;
    }
   
}


//根据指定的groupid查询群组的用户id列表 除userid自己   用于群组送发送消息业务
vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    //1.组装sql语句             c_str() string->char*
    char sql[1024] = {0};
    sprintf(sql,"select userid from groupuser where groupid = %d and userid != %d", groupid, userid);
    
    vector<int> idVec;
    MySQL mysql;
    if(mysql.connect())
    { 
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        { //查询成功
            MYSQL_ROW row;//获取该id对应的一行数据 row
            while((row = mysql_fetch_row(res))!= nullptr)
            {
                idVec.push_back(atoi(row[0]));
                
            }
            mysql_free_result(res);
        }
    }
    return idVec;
}