#pragma once
#include <string>

class User
{
    private:
        std::string name_;

};

//유저그룹 상속
class UserGroup : public User
{
    private:
        std::vector<User> users_;

};
