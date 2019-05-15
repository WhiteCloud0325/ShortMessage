#ifndef Protocol_H_
#define Protocol_H_
#include <string>
#include <stdint.h>
#include <time.h>
#include <vector>

enum SignalType {
    Login = 0x00,
    Logout = 0x02,
    MOBILE_MANAGER_REQUEST = 0x03,
    USER_QUERY = 0x20,
    GROUP_CREATE_REQUEST = 0x30,
    GROUP_CREATE_RESPONSE = 0x31,
    GROUP_ADD_MEMBER_REQUEST = 0x32,
    GROUP_ADD_MEMBER_RESPONSE = 0x33,
    GROUP_DELETE_MEMBER_REQUEST = 0x34,
    GROUP_DELETE_MEMBER_RESPONSE = 0X35,
    GROUP_QUIT_REQUEST = 0x36,
    GROUP_QUIT_RESPONSE = 0x37,
    GROUP_MESSAGE_REQUEST = 0x38,
    SIMPLE_MESSAGE = 0x40,
    FORWARD_NOACK_MESSAGE = 0x50,
    BACKWARD_NOACK_MESSAGE = 0X51,
    COMPLETE_MESSAGE = 0X60,
    INQUIRE_MESSAGE_REQUEST = 0X70,
    RECEIPTE = 0X80
};
/*
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
};*/

#pragma pack(push) 
#pragma pack(1)
struct ControlHead {
    uint32_t from_id;
    uint32_t to_id;
    uint16_t frame_id;
    uint8_t type;
    uint8_t retain;
    char content[]; 
};

// mobile 请求，开机，关机，移动性管理
struct MobileRequest {
    uint32_t timestamp;
    uint8_t position;   //位置0x00：东经南纬，0x01：东经北纬0x10：西经南纬，0x11：西经北纬
    float lng;  //经度  
    float lat;  //纬度
    float height;   //高度  
    char content[];  //cover
};

struct SateParam{
    SateParam():sate_id(0), beam_id(0), snr(0){}
    SateParam(int32_t sate_id_, int32_t beam_id_, int32_t snr_): sate_id(sate_id_), beam_id(beam_id_), snr(snr_){}
    SateParam(const SateParam &rhs) {
        this->sate_id = rhs.sate_id;
        this->beam_id = rhs.beam_id;
        this->snr = rhs.snr;
    }
    int32_t sate_id;
    int32_t beam_id;
    float snr;
};

struct Satellite{
    uint8_t grand_cover; //地面覆盖 0x00：无地面网络覆盖,0x01：有地面网络覆盖
    uint8_t sat_cover_num;
    std::vector<SateParam> sates_param;
};

struct Response {
    uint32_t to_id;
    uint32_t from_id;
    uint16_t frame_id;
    uint8_t type;
    uint8_t retain;
    uint8_t receipt_type;
    uint8_t receipt_indicate;
};


struct MessageReceiptItem{
    uint32_t user_id;
    uint16_t frame_id;
    uint8_t receipt_type;
    uint8_t receipt_indicate;
};
struct MessageResponse {
    uint64_t to_id;
    uint64_t from_id;
    uint16_t frame_id;
    uint8_t type;
    uint8_t retain;  
    MessageReceiptItem message_receipt_item;
    //std::vector<MessageReceiptItem> message_receipt_items; 
};

struct UserAckResponse {
    uint16_t frame_id;
    uint8_t receipt_type;
    uint8_t receipt_indicate;
};

struct MesaageInquireRequest {
    uint8_t num;
    uint32_t start_time;
    uint32_t end_time;
};


#pragma pack(pop)//恢复对齐状态


struct MessageItem {
    uint32_t to_id;
    uint32_t from_id;
    uint16_t frame_id;
    uint8_t type;
    uint8_t retain;
    std::string content;
};

struct InquireMessageItem {
    InquireMessageItem(){}
    InquireMessageItem(uint32_t from_id_, uint32_t timestamp_, const char * content_):from_id(from_id_), timestamp(timestamp_), content(content_){}
    uint32_t from_id;
    uint32_t timestamp;
    std::string content;
};

struct UserInfo {
    UserInfo(){}
    UserInfo(uint32_t user_id_, std::string user_name_): user_id(user_id_), user_name(user_name_) {}
    UserInfo(uint32_t user_id_, const char * user_name_): user_id(user_id_), user_name(user_name_) {}
    uint32_t user_id;
    std::string user_name;
};

struct GroupInfo{
    GroupInfo() {}
    GroupInfo(uint32_t group_id_, std::string group_name_): group_id(group_id_), group_name(group_name_) {} 
    GroupInfo(uint32_t group_id_, const char * group_name_): group_id(group_id_), group_name(group_name_) {}
    uint32_t group_id;
    std::string group_name;
};

struct GroupMessage {
    GroupMessage(){}
    GroupMessage(std::string content_, time_t timestamp_): content(content_), timestamp(timestamp_) {}
    GroupMessage(const char * content_, time_t timestamp_): content(content_), timestamp(timestamp_) {}
    std::string content;
    time_t timestamp;
};

#endif /* define Protocol*/
