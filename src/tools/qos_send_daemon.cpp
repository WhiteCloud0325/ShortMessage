#include "tools/qos_send_daemon.h"
#include "tools/logger.h"
#include "logic/coder.h"
#include "workmanager/send_helper.h"
#include <time.h>
#include <unistd.h>
#include <vector>
QosSendDaemon::QosSendDaemon(): stop_(false),
                                max_retry_count_(0),
                                check_interval_(0),
                                msg_interval_(0),
                                database_(NULL){

}

QosSendDaemon::~QosSendDaemon() {
    if (!stop_) {
        stop_ = true;
    }
}

bool QosSendDaemon::Init(const libconfig::Setting &setting, Database *database) {
    try {
        if (!setting.lookupValue("max_retry_count", max_retry_count_)) {
            return false;
        }
        if (!setting.lookupValue("check_interval", check_interval_)) {
            return false;
        }
        if (!setting.lookupValue("msg_interval", msg_interval_)) {
            return false;
        }
        stop_ = false;
    }
    catch(...) {
        LOG_INFO("QosSendDaemon Init Exception");
        return false;
    }
    database_ = database;
    return true;
}

void QosSendDaemon::Start() {
    while(!stop_) {
        Connection_T conn = database_->GetConnection();
        std::vector<MessageItem> messages;
        database_->GetAllOfflineMessage(conn, max_retry_count_, msg_interval_, messages);
        for(auto &message: messages) {
            std::vector<int32_t> beams = database_->GetSateCover(conn, message.to_id);
            if (beams.empty()) {
                continue;
            }
            std::string str = MessageEncode(message);
            SendHelper::GetInstance()->SendMessage(message.to_id, str, beams);
        }
        Connection_close(conn);
        sleep(check_interval_);
    }
}

void QosSendDaemon::Stop() {
    stop_ = true;
}




