#include "logic/user_manager.h"
#include "proto/im.pb.h"
#include "tools/logger.h"
using namespace im;

UserManager::~UserManager(){}

bool UserManager::Init(Database *database){
    database_ = database;
    return true;
}


 
