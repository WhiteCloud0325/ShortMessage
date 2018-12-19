#ifndef Protocol_H_
#define Protocol_H_
#include <string>
#include <stdint.h>
#include <time.h>
struct Protocol {
    Protocol(): type(0), uid(-1), net_type(-1), ip(0), retry_count(0){}
    Protocol(int32_t type_, int64_t uid_, int32_t net_type_, uint32_t ip_, int64_t msg_id_, std::string content_):
        type(type_), uid(uid_), net_type(net_type_), ip(ip_), 
        msg_id(msg_id_), data_content(content_), retry_count(0){
            timestamp = time(NULL);
        }
    Protocol(const Protocol &rhs) {
        type = rhs.type;
        uid = rhs.uid;
        net_type = rhs.net_type;
        ip = rhs.ip;
        data_content = rhs.data_content;
        retry_count = rhs.retry_count;
    }
    int32_t type;
    int32_t net_type;
    int64_t uid;
    uint32_t ip;
    int retry_count;
    int64_t msg_id;
    std::string data_content;
    time_t timestamp;
    

};


struct SendMessage {
    SendMessage(){}
    SendMessage(int64_t from_id_, int64_t to_id_, int64_t timestamp_): msg_id(0), from_id(from_id_), to_id(to_id_), timestamp(timestamp_) {}
    SendMessage(const SendMessage& rhs) {
        msg_id = rhs.msg_id;
        from_id = rhs.from_id;
        to_id = rhs.to_id;
        content = rhs.content;
        timestamp = rhs.timestamp;
    }
    int64_t msg_id;
    int64_t from_id;
    int64_t to_id;
    std::string content;
    int64_t timestamp;
};
#endif /* define Protocol*/