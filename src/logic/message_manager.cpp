#include "logic/message_manager.h"

bool MessageManager::Init(Database* database) {
    database_ = database;
    return true;
}