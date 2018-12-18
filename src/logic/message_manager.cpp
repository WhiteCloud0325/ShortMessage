#include "logic/message_manager.h"
#include "proto/im.pb.h"
#include "tools/logger.h"
#include "workmanager/send_helper.h"
#include <time.h>
using namespace im;

bool MessageManager::Init(Database* database, QosSendDaemon *qos_send_daemon) {
    database_ = database;
    qos_send_daemon_ = qos_send_daemon;
    return true;
}

void MessageManager::ProcessC2SMessage(const std::string &request) {
    im::MessageItem message_item;
    if (!message_item.ParseFromString(request)) return;
    int64_t from_id = message_item.fromid();
    int64_t to_id = message_item.toid();
    std::string content = message_item.content();
    time_t timestamp = message_item.timestamp();
    Connection_T conn = database_->GetConnection();
    int64_t msg_id = database_->InsertMessage(conn, from_id, to_id, content, timestamp);
    if (msg_id < 0) {
        return;
    }
    message_item.set_msgid(msg_id);
    std::string message;
    message_item.SerializeToString(&message);
    if (database_->UserOnline(conn, to_id)) {
        Protocol p;
        p.type = MESSAGE_FRIEND_SEND_REQUEST;
        p.net_type = 1;
        p.msg_id = msg_id;
        p.uid = to_id;
        p.data_content = message;
        p.timestamp = time(NULL);
        qos_send_daemon_->Put(p);
        SendHelper::GetInstance()->SendMessageData(p);
    }
}

void MessageManager::ProcessACK(const std::string &ack) {
    MessageResponse response;
    response.ParseFromString(ack);
    int64_t msg_id = response.msgid();
    Connection_T conn = database_->GetConnection();
    database_->SetStateMessage(conn, msg_id);
    qos_send_daemon_->Remove(msg_id);
    LOG_DEBUG("Message ACK Success: msg_id=%lld", msg_id);
}
