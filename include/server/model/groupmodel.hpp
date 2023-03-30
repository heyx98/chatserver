#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.hpp"

//group表的数据操作类     sql的api调用  封装在这个类中
class GroupModel{
private:
public:
    //创建群组
    bool creatGroup(Group &group);

    //加入群组
    void addGroup(int userid, int groupid, string role);

    //查询用户所在群组信息
    vector<Group> queryGroup(int userid);

    //根据指定的groupid查询群组的用户id列表 除userid自己   用于群组送发送消息业务
    vector<int> queryGroupUsers(int userid, int groupid);

};

#endif