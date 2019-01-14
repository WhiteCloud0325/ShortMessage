#ifndef USER_MANAGER_H_
#define USER_MANAGER_H_

#include <string>
#include "tools/database.h"


class UserManager  {
public:
    UserManager():database_(NULL){}
    ~UserManager();
    bool Init(Database *database);
private:
    Database *database_;
};

#endif /* define USER_MANAGER_H_*/
