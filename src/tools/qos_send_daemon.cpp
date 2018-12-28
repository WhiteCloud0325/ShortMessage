#include "tools/qos_send_daemon.h"
#include "tools/logger.h"
#include "logic/coder.h"
#include "workmanager/send_helper.h"
#include <time.h>
#include <unistd.h>
#include <vector>
#include <arpa/inet.h>
#include <iostream>
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
            uint32_t user_id = message.to_id;
            std::vector<SateParam> sates = database_->GetSateCover(conn, user_id);
            if (sates.empty()) {
                continue;
            }
            //std::cout << user_id << std::endl;
            std::string str = MessageEncode(message);
            SendHelper::GetInstance()->SendMessage(message.to_id, str, sates, 5);
        }
        Connection_close(conn);
        sleep(check_interval_);
    }
}

void QosSendDaemon::Stop() {
    stop_ = true;
}




