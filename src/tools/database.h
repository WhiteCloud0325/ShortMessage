#ifndef DATABASE_H_
#define DATABASE_H_

#include <zdb.h>
#include <libconfig.h++>
#include <stdint.h>
#include <string>
#include <vector>
#include <time.h>
#include "logic/protocol.h"


class Database {
public:
    Database(){}
    bool Init(const libconfig::Setting &setting);
    ~Database();
    Connection_T GetConnection();
    bool UpdateSateCover(Connection_T conn, const uint32_t &user_id, const Satellite& sate_cover);
    void GetSateCover(Connection_T conn, const int32_t &user_id, std::vector<SateParam> &res);
    int GetOfflineMessageNum(Connection_T conn, uint32_t &user_id);
    bool InsertStoreMessage(Connection_T conn, ControlHead *control_head);
    bool InsertStoreMessage(Connection_T conn, const MessageItem &msg, const time_t &recv_time);
    bool InsertOfflineMessage(Connection_T conn, ControlHead *control_head);
    bool DeleteOfflineMessage(Connection_T conn, const uint32_t &from_id, const uint32_t &to_id, const uint16_t frame_id);
    void UpdateOfflineMessage(Connection_T conn, const uint32_t &from_id, const uint32_t &to_id, const uint16_t frame_id, const int &retry_num, const time_t &timestamp);
    void GetAllOfflineMessage(Connection_T conn, const int &max_retry_num, const time_t &retry_interval, std::vector<MessageItem> &messages);
    bool IsExistUser(Connection_T conn, const uint32_t &user_id);
    bool GetOfflineMessage(Connection_T conn, const uint32_t &from_id, const uint32_t &to_id, const uint16_t &frame_id, MessageItem &msg, time_t &recv_time);
    bool FriendGet(Connection_T conn, const uint32_t &user_id, const uint32_t &friend_id);
    bool FriendAdd(Connection_T conn, const uint32_t &user_id, const uint32_t &friend_id);
    bool FriendDelete(Connection_T conn, const uint32_t &user_id, const uint32_t &friend_id);
    bool FriendList(Connection_T conn, const uint32_t &user_id, std::vector<UserInfo> &friends);
    int GroupCreate(Connection_T conn, const uint32_t &user_id, const std::string &group_name, std::vector<uint32_t> &members);
    bool GroupAddMember(Connection_T conn, const uint32_t &group_id, const  std::vector<uint32_t> &members);
    bool GroupDeleteMember(Connection_T conn, const uint32_t &group_id, const uint32_t &member);
    bool GroupDeleteMembers(Connection_T conn, const uint32_t &group_id, const std::vector<uint32_t> &members);
    bool GroupListByUserId(Connection_T conn, const uint32_t &user_id, std::vector<GroupInfo> &groups);
    bool GroupListUser(Connection_T conn, const uint32_t &group_id, std::vector<UserInfo> &user_infos);
    std::vector<uint32_t> GroupListUserId(Connection_T conn, const uint32_t &group_id);
    int64_t GroupMessageInsert(Connection_T conn, const uint32_t &group_id, const uint32_t &user_id, const char* content, const time_t &recv_time);
    bool GroupMessageUpdate(Connection_T conn, const uint32_t &group_id, const uint32_t &user_id, const uint64_t &msg_id);
    int GroupMessagePull(Connection_T conn, const uint32_t &group_id, const uint32_t &user_id, std::vector<GroupMessage> &messages);
    int  IsUserInGroup(Connection_T conn, const uint32_t &group_id, const uint32_t &user_id);
private:
    std::string host_name_;
    std::string user_;
    std::string password_;
    std::string db_name_;
    URL_T mysql_url_;
    ConnectionPool_T pool_;
    int max_size_;
};


#endif  /* define database_h*/
