#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "user.hpp"
using namespace std;

//群组用户 继承自User类 多了一个用户在群组中的角色信息
class GroupUser : public User
{
public:
    void setRole(string role){this->role = role;}
    string getRole() {return this->role;}
    
    
private:
    string role;   //用户在群组中的角色信息
};


#endif