#ifndef USER_MANAGER_H_
#define USER_MANAGER_H_

#include <string>
#include "logic/manager.h"

class UserManager: public Manager {
public:
    UserManager(){}
    ~UserManager(){};
    void Register(const ControlHead * control_head, Connection_T conn);
    void FriendAdd(const ControlHead * control_head, Connection_T conn);
    void FriendDelete(const ControlHead * control_head, Connection_T conn);
    void ListUserFriend(const ControlHead * control_head, Connection_T conn);
    void GroupCreate(const ControlHead * control_head, Connection_T conn);
    void GroupAddUser(const ControlHead * control_head, Connecton_T conn);

};

#endif /* define USER_MANAGER_H_*/
