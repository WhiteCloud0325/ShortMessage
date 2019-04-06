#include "logic/message_manager.h"
#include "tools/logger.h"
#include "logic/coder.h"
#include "workmanager/send_helper.h"
#include <time.h>
#include <arpa/inet.h>
using namespace im;

bool MessageManager::Init(Database *database) {
    database_ = database;
    return true;
}

/**
 *  function: ProcessSimpleMessage
 *  处理 0x40消息，不需要向发送方确认，也不需要接收方确认，
 *  所以不用重传，消息只存入message_store
 */
void MessageManager::ProcessSimpleMessage(ControlHead *control_head, Connection_T conn) {
    uint32_t to_id = ntohl(control_head->to_id);
    uint32_t from_id = ntohl(control_head->from_id);
    if (!database_->InsertStoreMessage(conn, control_head)) {
        return;
    }
    std::vector<SateParam> sates = database_->GetSateCover(conn, to_id);
    if (!sates.empty()) {
        std::string str = MessageEncode(control_head);
        SendHelper::GetInstance()->SendMessage(to_id, str, sates, 5);
        LOG_DEBUG("Message ProcessCompleteMessage SendMessage: from_id=%ld||to_id=%ld||frame_id=%ld", from_id, to_id, ntohs(control_head->frame_id));
    }
    return;
}

void MessageManager::ProcessForwardNoAckMessage(ControlHead *control_head, Connection_T conn) {
}
void MessageManager::ProcessBackwardNoAckMessage(ControlHead *control_head, Connection_T conn) {
}
void MessageManager::ProcessCompleteMessage(ControlHead *control_head, Connection_T conn) {
    uint32_t to_id = ntohl(control_head->to_id);
    uint32_t from_id = ntohl(control_head->from_id);
    if (!database_->InsertOfflineMessage(conn, control_head)) {
        return;
    }
    std::vector<SateParam> sates_to_user = database_->GetSateCover(conn, to_id);
    std::vector<SateParam> sates_from_user = database_->GetSateCover(conn, from_id);
    Response response;
    response.to_id = htonl(from_id);
    response.from_id = 0;
    response.frame_id = control_head->frame_id;
    response.type = 0x80;
    response.retain = 0;
    response.receipt_type = 0x50;
    response.receipt_indicate = 0;
    if (!sates_from_user.empty()) {
        std::string res = ResponseEncode(response);
        SendHelper::GetInstance()->SendMessage(from_id, res, sates_from_user, 10);
        LOG_DEBUG("Message ProcessCompleteMessage SendReceipt: from_id=%ld", from_id);
    }
    if (!sates_to_user.empty()) {
        std::string str = MessageEncode(control_head);
        SendHelper::GetInstance()->SendMessage(to_id, str, sates_to_user, 5);
        LOG_DEBUG("Message ProcessCompleteMessage SendMessage: from_id=%ld||to_id=%ld||frame_id=%ld", from_id, to_id, ntohs(control_head->frame_id));
    }
    return;
}

void MessageManager::ProcessReceipt(ControlHead *control_head, Connection_T conn) {
    uint32_t to_id = ntohl(control_head->from_id);
    uint32_t from_id = ntohl(control_head->to_id);
    UserAckResponse *user_ack_response = (UserAckResponse*) control_head->content;
    if (user_ack_response->receipt_type != 0x60) {
        return;
    }
    uint16_t frame_id = ntohs(user_ack_response->frame_id);
    MessageItem msg;
    time_t recv_time;
    if(database_->GetOfflineMessage(conn, from_id, to_id, frame_id, msg, recv_time) && msg.type == 0x60) {
        database_->DeleteOfflineMessage(conn, from_id, to_id, frame_id);
        LOG_DEBUG("Message Receipt: from_id=%ld||to_id=%ld||frame_id=%ld", from_id, to_id, frame_id);
        database_->InsertStoreMessage(conn, msg, recv_time);
        /* MessageResponse response;
        response.to_id = htonl(from_id);
        response.from_id = 0;
        response.frame_id = htons(frame_id + 1);
        response.type = 0x80;
        response.retain = 0;
        response.message_receipt_item.user_id = htonl(to_id);
        response.message_receipt_item.frame_id = htons(frame_id);
        response.message_receipt_item.type = 0x60;
        response.message_receipt_item.retain = 0;
        std::string str = ResponseEncode(response);
        SendHelper::GetInstance()->SendMessage(from_id, str, ,10);*/
    }
    return;
}
