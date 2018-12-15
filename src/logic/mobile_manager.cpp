#include "logic/mobile_manager.h"
#include "workmanager/send_helper.h"
#include "proto/im.pb.h"

using namespace im;

bool MobileManager::Init(Database *database) {
    database_ = database;
    return true;
}


