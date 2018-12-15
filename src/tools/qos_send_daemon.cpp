#include "tools/qos_send_daemon.h"
#include "tools/logger.h"
#include <time.h>
#include <vector>
QosSendDaemon::QosSendDaemon(): stop_(false),
                                max_retry_count_(0),
                                check_interval_(0),
                                msg_interval_(0){

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
        std::vector<int64_t> lost_messages;
        for (msg_hash_map::iterator it = send_messages_.begin(); it != send_messages_.end(); ++it) {
            int64_t msg_id = it->first;
            Protocol &p = it->second;
            
            if (p.retry_count >= max_retry_count_) {
                LOG_INFO("QosSendDaemon message retry failed: msg_id=%lld||uid=%lld", msg_id, p.uid);
                lost_messages.push_back(msg_id);
            }
            else {
                time_t delta = time(NULL) - p.timestamp;
                if (delta > msg_interval_) {
                    //Todo
                    p.retry_count++;
                    p.timestamp = time(NULL);
                    LOG_DEBUG("QosSendDaemon message retry: msg_id=%lld||retry=%d", msg_id, p.retry_count);
                }
            }
        }
        for (auto i : lost_messages) {
            send_messages_.erase(i);
        }
        sleep(check_interval_);
    }
}

void QosSendDaemon::Stop() {
    stop_ = true;
}

bool QosSendDaemon::Put(Protocol &p) {
    int64_t msg_id = p.msg_id;
    msg_hash_map::accessor result;
    if (send_messages_.find(result, msg_id)) {
        return false;
    }
    if (!send_messages_.insert(result, msg_id)) {
        return false;
    }
    result->second = p;
    return true;
}

bool QosSendDaemon::Remove(int64_t key) {
    if (send_messages_.erase(key)) {
        return true;
    }
    else {
        return false;
    }
}


