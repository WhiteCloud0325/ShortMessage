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
    std::vector<SateParam>  GetSateCover(Connection_T conn, const int32_t &user_id);
    int GetOfflineMessageNum(Connection_T conn, uint32_t &user_id);
    bool InsertStoreMessage(Connection_T conn, ControlHead *control_head);
    bool InsertStoreMessage(Connection_T conn, const MessageItem &msg, const time_t &recv_time);
    bool InsertOfflineMessage(Connection_T conn, ControlHead *control_head);
    bool DeleteOfflineMessage(Connection_T conn, const uint32_t &from_id, const uint32_t &to_id, const uint16_t frame_id);
    void UpdateOfflineMessage(Connection_T conn, const uint32_t &from_id, const uint32_t &to_id, const uint16_t frame_id, const int &retry_num, const time_t &timestamp);
    void GetAllOfflineMessage(Connection_T conn, const int &max_retry_num, const time_t &retry_interval, std::vector<MessageItem> &messages);
    bool IsExistUser(Connection_T conn, const uint32_t &user_id);
    bool GetOfflineMessage(Connection_T conn, const uint32_t &from_id, const uint32_t &to_id, const uint16_t &frame_id, MessageItem &msg, time_t &recv_time);
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
