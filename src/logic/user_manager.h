#ifndef USER_MANAGER_H_
#define USER_MANAGER_H_

#include <string>
#include "logic/manager.h"
#include "tools/database.h"

class UserManager: public Manager {
public:
    UserManager(){}
    ~UserManager(){};
    void Register(ControlHead * control_head, Connection_T conn);
    void FriendAdd(ControlHead * control_head, Connection_T conn);
    void FriendList(ControlHead * control_head, Connection_T conn);
    void FriendDelete(ControlHead * control_head, Connection_T conn);
    void GroupCreate(ControlHead * control_head, Connection_T conn);
    void GroupAddUser(ControlHead * control_head, Connection_T conn);
    void GroupQuitUser(ControlHead * control_head, Connection_T conn);
    void GroupDeleteMember(ControlHead * control_head, Connection_T conn);
};

#endif /* define USER_MANAGER_H_*/
