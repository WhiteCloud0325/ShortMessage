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
    bool UpdateSateCover(Connection_T conn, const int64_t &user_id, const Satellite& sate_cover);
    std::vector<uint8_t>  SelectSateCover(Connection_T conn, const int64_t &user_id);
    int64_t InsertMessage(Connection_T conn, const int64_t &from_id, const int64_t &to_id, const std::string &content, const time_t &timestamp);
    void GetOfflineMessage(Connection_T conn, const int64_t &to_id, int limit_num);
    bool SetStateMessage(Connection_T conn, const int64_t &msg_id);
    bool UserOnline(Connection_T conn, const int64_t &user_id);
private:
    std::string host_name_;
    std::string user_;
    std::string password_;
    std::string db_name_;
    URL_T mysql_url_;
    ConnectionPool_T pool_;
};


#endif  /* define database_h*/
