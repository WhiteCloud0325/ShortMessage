#include "logic/mobile_manager.h"
#include "proto/im.pb.h"

using namespace im;

bool MobileManager::Init(Database *database, QosSendDaemon *qos_send_daemon) {
    database_ = database;
    qos_send_daemon_ = qos_send_daemon;
    return true;
}

void MobileManager::ProcessC2sMessage(const std::string &request) {
    im::MessageItem message_item;
    if (!message_item.ParseFromString(reqeust)) return;
    int64_t from_id = message_item.fromid();
    int64_t to_id = message_item.toid();
    std::string content = message_item.content();
    time_t timestamp = message_item.timestamp();
    Connetion_T conn = database_->GetConnection();
    int64_t msg_id = database_->InsertMessage(conn, from_id, to_id, content, timestamp);
    if (msg_id < 0) {
        return;
    }
    if (database_->UserOnline(conn, to_id)) {
        Protocol p;
        p.type = MESSAGE_FRIEND_SEND_REQUEST;
        p.net_type = 1;
        p.uid = to_id;
        p.data_content = content;
        p.timestamp = time(NULL);
        qos_send_daemon_.Put(p);
        SendHelper::GetInstance()->SendMessageData(p);
    }
}