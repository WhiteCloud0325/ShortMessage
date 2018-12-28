#include "logic/message_manager.h"
#include "tools/logger.h"
#include "logic/coder.h"
#include "workmanager/send_helper.h"
#include <time.h>
#include <arpa/inet.h>
using namespace im;

bool MessageManager::Init(Database* database) {
    database_ = database;
    return true;
}

/**
 *  function: ProcessSimpleMessage
 *  处理 0x40消息，不需要向发送方确认，也不需要接收方确认，
 *  所以不用重传，消息只存入message_store
 */
void MessageManager::ProcessSimpleMessage(ControlHead *control_head) {
    uint32_t to_id = ntohl(control_head->to_id);
    uint32_t from_id = ntohl(control_head->from_id);


    Connection_T conn = database_->GetConnection();
    if (conn == NULL) {
        return;
    }
    int64_t id = database_->InsertStoreMessage(conn, control_head);
    if (id == -1) {
        Connection_close(conn);
        return;
    }
    std::vector<SateParam> sates = database_->GetSateCover(conn, to_id);
    Connection_close(conn);
    if (!sates.empty()) {
        std::string str = MessageEncode(control_head);
        SendHelper::GetInstance()->SendMessage(to_id, str, sates, 5);
    }
    
    return;
}

void MessageManager::ProcessForwardNoAckMessage(ControlHead *control_head) {
}
void MessageManager::ProcessBackwardNoAckMessage(ControlHead *control_head) {
}
void MessageManager::ProcessCompleteMessage(ControlHead *control_head) {
    uint32_t to_id = ntohl(control_head->to_id);

    Connection_T conn = database_->GetConnection();
    if (conn == NULL) {
        return;
    }

    int64_t id = database_->InsertStoreMessage(conn, control_head);
    database_->InsertOfflineMessage(conn, control_head, id);
    std::vector<SateParam> sates = database_->GetSateCover(conn, to_id);
    Connection_close(conn);
    if (!sates.empty()) {
        std::string str = MessageEncode(control_head);
        SendHelper::GetInstance()->SendMessage(to_id, str, sates, 5);
    }
    return;
}
