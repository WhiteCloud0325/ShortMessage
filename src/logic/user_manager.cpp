#include "logic/user_manager.h"
#include "tools/logger.h"

UserManager::~UserManager(){}

bool UserManager::Init(Database *database){
    database_ = database;
    return true;
}


 
