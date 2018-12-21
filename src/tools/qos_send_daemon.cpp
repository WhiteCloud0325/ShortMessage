#include "tools/qos_send_daemon.h"
#include "tools/logger.h"
#include <time.h>
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

bool QosSendDaemon::Init(const libconfig::Setting &setting) {
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
    return true;
}

void QosSendDaemon::Start() {
    while(!stop_) {
        
        sleep(check_interval_);
    }
}

void QosSendDaemon::Stop() {
    stop_ = true;
}




