#include "tools/database.h"
#include "tools/logger.h"
#include <arpa/inet.h>

const int MAX_BEAM_NUM = 10;
bool Database::Init(const libconfig::Setting &setting) {
    try {
        if (!setting.lookupValue("host", host_name_)) {
            LOG_INFO("Database init config host error");
            return false;
        }
        if (!setting.lookupValue("user", user_)) {
            LOG_INFO("Database init config user error");
            return false;
        }
        if (!setting.lookupValue("password", password_)) {
            LOG_INFO("Database init config password error");
            return false;
        }
        if (!setting.lookupValue("db", db_name_)) {
            LOG_INFO("Database init config db error");
            return false;
        }
        if (!setting.lookupValue("max_size", max_size_)) {
            LOG_INFO("Dababase init config max_size error");
            return false;
        }
    }
    catch(...) {
        LOG_INFO("Database init config exception");
        return false;
    }
    try {
        char url[1024];
        sprintf(url, "mysql://%s/%s?user=%s&password=%s", host_name_.c_str(), db_name_.c_str(), user_.c_str(), password_.c_str());
        mysql_url_ = URL_new(url);
        pool_ = ConnectionPool_new(mysql_url_);
        ConnectionPool_setMaxConnections(pool_, max_size_);
        ConnectionPool_start(pool_);
    }
    catch (...) {
        LOG_INFO("Database init ConnectionPool start exception");
        return false;
    }
    return true;
}

Database::~Database() {
    ConnectionPool_free(&pool_);
    URL_free(&mysql_url_);
}

/*void Database::Query() {
    Connection_T conn = ConnectionPool_getConnection(pool_);
    if (conn) {
        ResultSet_T result = Connection_executeQuery(conn, "select id, name, tel from students");
        while (ResultSet_next(result)) {
            int id = ResultSet_getInt(result, 1);
            const char* name = ResultSet_getString(result, 2);
            const char* tel = ResultSet_getString(result, 3);
            printf("id = %d, name = %s, tel = %s\n", id, name, tel);
        }
        Connection_close(conn);
    }
}*/

Connection_T Database::GetConnection() {
    int count = 3;
    Connection_T conn = NULL;
    while (count > 0) {
        conn = ConnectionPool_getConnection(pool_);
        if (conn) {
            return conn;
        }
    }
    return conn;
}

/** UpdateNetworCover
 * function: 设置networkCover net_type 为1
 * params：
 *          @conn, Connetcion_T
 *          @user_id, uint32_t
 *          @sate_cover, Satellite
 * return: bool, success true, fail false
*/
bool Database::UpdateSateCover(Connection_T conn, const uint32_t &user_id, const Satellite& sate_cover) {
    std::vector<float> temp_snr(MAX_BEAM_NUM, -1);
    for (int i = 0; i < sate_cover.sat_cover_num; ++i) {
        assert(sate_cover.sates_param[i].beam_id >=1 && sate_cover.sates_param[i].beam_id <= 10);
        temp_snr[sate_cover.sates_param[i].beam_id - 1] = sate_cover.sates_param[i].snr;
    } 
    std::string sql = "UPDATE sate_cover SET ";
    for (int i = 1; i <= MAX_BEAM_NUM; ++i) {
        sql += "SNR_" + std::to_string(i) + " = " + std::to_string(temp_snr[i - 1]) + ",";
    }
    sql.pop_back();
    sql += " WHERE user_id = " + std::to_string(user_id);
    TRY {
        Connection_execute(conn, sql.c_str());
    }
    CATCH(SQLException) {
        LOG_INFO("Database UpdateSateCover Failed: userid=%lld||SQLException=%s", user_id, Connection_getLastError(conn));
        return false;
    }
    END_TRY;
    return true;
}

/**
 *  function: GetSateCover
 *  params:
 *          @conn: Connection_T
 *          @user_id: uint32_t
 *  return: vector<int32_t> 一组波束id 
 *  注：考虑到thrift 列表时vector<int32_t> 为了方便这里也使用vector<int32_t>
 */


std::vector<SateParam> Database::GetSateCover(Connection_T conn, const int32_t &user_id) {
    std::vector<SateParam> res; 
    std::string sql = "SELECT SNR_1, SNR_2, SNR_3, SNR_4, SNR_5, SNR_6, SNR_7, SNR_8, SNR_9, SNR_10	 FROM sate_cover WHERE user_id = ";
    sql += std::to_string(user_id);
    TRY {
        ResultSet_T r = Connection_executeQuery(conn, sql.c_str());
        if (ResultSet_next(r)) {
             for (int i = 1; i <= MAX_BEAM_NUM; ++i) {
                 double temp = ResultSet_getDouble(r, i);
                 if (temp > 0) {
                     res.push_back(SateParam((i + 1) / 2, i, (float)temp));
                 }
             }
        }
    }
    CATCH(SQLException) {
        LOG_INFO("Database SelectSateCover Failed: user_id=%lld||SQLException=%s", user_id, Connection_getLastError(conn));
    }
    END_TRY;
    return res;
}

/**
 *  function: GetOfflineMessageNum 从表message_cache获取to_user的离线消息数目
 *  params：
 *          @conn, Conneciont_T
 *          @user_id, Connection_T conn
 *  return: int ， 异常等都为0
 */

int Database::GetOfflineMessageNum(Connection_T conn, uint32_t &user_id) {
    int res = 0;
    std::string sql = "SELECT COUNT(*) FROM message_cache WHERE to_user = ";
    sql += std::to_string(user_id);
    TRY {
        ResultSet_T r = Connection_executeQuery(conn, sql.c_str());
        if (ResultSet_next(r)) {
            res = ResultSet_getInt(r, 1);
        }
    }
    CATCH(SQLException) {
        LOG_INFO("Database GetOfflineMessageNum Failed: user_id=%lld||SQLException=%s", user_id, Connection_getLastError(conn));
    }
    END_TRY;
    return res;
}

/**
 *  function: InsertStoreMessage 消息存储
 *  params：
 *          @conn, Connection_T
 *          @control_head, ControlHead
 *  return: bool
 */ 
bool Database::InsertStoreMessage(Connection_T conn, ControlHead* control_head) {
    if (control_head == NULL) {
        return false;
    }
    char sql[4096] = {0};
    uint32_t to_id = ntohl(control_head->to_id);
    uint32_t from_id = ntohl(control_head->from_id);
    uint16_t frame_id = ntohs(control_head->frame_id);
    time_t recv_time = time(NULL);
    tm tm_recv_time;
    gmtime_r(&recv_time, &tm_recv_time);
    char timestamp[64] = {0};
    strftime(buf, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm_recv_time);
    sprintf(sql, "INSERT INTO message_store(from_user, to_user, text, recv_time, send_time, frame_id, type) VALUES(%d, %d, '%s', '%s', '%s', %d, %d)",
            from_id, to_id, control_head->content, timestamp, timestamp, frame_id, control_head->type);
    TRY{
        Connection_execute(conn, sql);
        //id = Connection_lastRowId(conn);
    }
    CATCH(SQLException) {
        LOG_INFO("Database InsertStoreMessage Failed: from_id=%u||to_id=%u||frame_id=%u||SQLException=%s", from_id, to_id, frame_id, Connection_getLastError(conn));
        return false;
    }
    END_TRY;
    return true;
}

/**
 *  function: InsertStoreMessage 消息存储
 *  params：
 *          @conn
 *          @msg, MessageItem
 *          @recv_time, time_t
 *  return: bool
 */ 
bool Database::InsertStoreMessage(Connection_T conn, const MessageItem &msg, const time_t &recv_time) {
    char sql[4096] = {0};
    time_t send_time = time(NULL);
    tm tm_recv_time, tm_send_time;
    gmtime_r(&recv_time, &tm_recv_time);
    gmtime_r(&send_time, &tm_send_time);
    char recv_time_buf[64] = {0};
    char send_time_buf[64] = {0};
    strftime(recv_time_buf, sizeof(recv_time_buf), "%Y-%m-%d %H:%M:%S", &tm_recv_time);
    strftime(send_time_buf, sizeof(send_time_buf), "%Y-%m-%d %H:%M:%S", &tm_send_time);
    sprintf(sql, "INSERT INTO message_store(from_user, to_user, type, text, recv_time, send_time, frame_id) VALUES(%d, %d, %d, '%s', '%s', '%s', %d)",
            msg.from_id, msg.to_id, msg.type, msg.content.c_str(), recv_time_buf, send_time_buf, msg.frame_id);
    TRY{
        Connection_execute(conn, sql);
    }
    CATCH(SQLException) {
        LOG_INFO("Database InsertStoreMessage Failed: from_id=%u||to_id=%u||frame_id=%u||SQLException=%s", msg.from_id, msg.to_id, msg.frame_id, Connection_getLastError(conn));
        return false;
    }
    END_TRY;
    return true;
}

/** function:InsertOfflineMessage 消息缓存
 *  params：
 *         @conn, Connetion)T
 *         @control_head, ControlHead
 *  return: bool
 */ 
bool Database::InsertOfflineMessage(Connection_T conn, ControlHead* control_head) {
    if (control_head == NULL) {
        return false;
    }
    char sql[2048] = {0};
    uint32_t to_id = ntohl(control_head->to_id);
    uint32_t from_id = ntohl(control_head->from_id);
    uint16_t frame_id = ntohs(control_head->frame_id);
    time_t timestamp = time(NULL);
    tm tm_recv_time, tm_retry_time;
    gmtime_r(&timestamp, &tm_recv_time);
    char recv_time_buf[64] = {0};  
    strftime(recv_time_buf, sizeof(recv_time_buf), "%Y-%m-%d %H:%M:%S", &tm_recv_time);
    timestamp += 10;
    gmtime_r(&timestamp, &tm_retry_time);  
    char retry_time_buf[64] = {0};
    strftime(retry_time_buf, sizeof(retry_time_buf), "%Y-%m-%d %H:%M:%S", &tm_retry_time);
    sprintf(sql, "INSERT INTO message_cache(from_user, to_user, type, text, recv_time, frame_id, retry_time) VALUES(%d, %d, %d, '%s', '%s', %d, '%s')",
            from_id, to_id, control_head->type, control_head->content, recv_time_buf, frame_id, retry_time_buf);
    TRY{
        Connection_execute(conn, sql);
    }
    CATCH(SQLException) {
        LOG_INFO("Database InsertOfflineMessage Failed: from_id=%u||to_id=%u||frame_id=%u||SQLException=%s", from_id, to_id, frame_id, Connection_getLastError(conn));
        return false;
    }
    END_TRY;
    return true;
}

/**
 *  function:  DeleteOfflineMessage
 *  params: 
 *          @conn
 *          @from_id
 *          @to_id
 *          @frame_id
 */ 
bool Database::DeleteOfflineMessage(Connection_T conn, const uint32_t &from_id, const uint32_t &to_id, const uint16_t frame_id) {
    char sql[1024] = {0};
    sprintf(sql, "DELETE FROM message_cache WHERE from_user = ? AND to_user = ? AND frame_id = ?",
            from_id, to_id, frame_id);
    TRY{
        Connection_execute(conn, sql);
    }
    CATCH(SQLException) {
        LOG_INFO("Database DeleteOfflineMessage Failed: from_id=%u||to_id=%u||frame_id=%u||SQLException=%s", from_id, to_id, frame_id, Connection_getLastError(conn));
        return false;
    }
    END_TRY;
    return true;
}

/**
 *  function UpdateOfflineMessage
 */ 
void Database::UpdateOfflineMessage(Connection_T conn, const uint32_t &from_id, const uint32_t &to_id, const uint16_t frame_id, const int &retry_num, const time_t &timestamp) {
    char sql[2048] = {0};
    tm tm_time;
    gmtime_r(&timestamp, &tm_time);
    char time_buf[64] = {0};
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &tm_time);
    sprintf(sql, "UPDATE message_cache SET retry_num = %d, retry_time = '%s' WHERE from_user = %d AND to_user = %d AND frame_id = %d",
            retry_num, time_buf, from_id, to_id, frame_id);
    TRY{
        Connection_execute(conn, sql);
    }
    CATCH(SQLException) {
        LOG_INFO("Database UpdateOfflineMessage Failed: from_id=%u||to_id=%u||frame_id=%u||SQLException=%s", from_id, to_id, frame_id, Connection_getLastError(conn));
    }
    END_TRY;
}

/**
 *  function: GetAllOfflineMessage 获取离线消息用于重传
 *  params：
 *          @conn, Connection_T
 *          @max_retry_num, 最大重试时间 int
 *          @time_t, 时间戳
 * 
 */
void Database::GetAllOfflineMessage(Connection_T conn, const int &max_retry_num, const time_t &retry_interval, std::vector<MessageItem> &messages) {
    time_t cur_timestamp = time(NULL);
    char sql[1024] = {0};
    tm tm_cur;
    gmtime_r(&cur_timestamp, &tm_cur);
    char cur_buf[64] = {0};
    strftime(cur_buf, sizeof(cur_buf), "%Y-%m-%d %H:%M:%S", &tm_cur);
    sprintf(sql, "SELECT to_user, from_user, frame_id, type, text, retry_num FROM message_cache WHERE retry_num < %d AND retry_time <= '%s'",
            max_retry_num, cur_buf);
    PreparedStatement_T p = Connection_prepareStatement(conn, "UPDATE message_cache SET retry_num = ?, retry_time = ? WHERE to_user = ? AND from_user = ? AND frame_id = ?");
    TRY{
        ResultSet_T r = Connection_executeQuery(conn, sql);
        while(ResultSet_next(r)) {
            MessageItem message;
            message.to_id = ResultSet_getInt(r, 1);
            message.from_id = ResultSet_getInt(r, 2);
            message.frame_id = ResultSet_getInt(r, 3);
            message.type = ResultSet_getInt(r, 4);
            message.retain = 0x00;
            message.content = std::string(ResultSet_getString(r, 5));
            int retry_num = ResultSet_getInt(r, 6);
            ++retry_num;
            messages.push_back(message);
            PreparedStatement_setInt(p, 1, retry_num);
            PreparedStatement_setTimestamp(p, 2, cur_timestamp + retry_num * retry_interval);
            PreparedStatement_setInt(p, 3, (int)message.to_id);
            PreparedStatement_setInt(p, 4, (int)message.from_id);
            PreparedStatement_setInt(p, 5, (int)message.frame_id);
            PreparedStatement_execute(p);
        }
    }
    CATCH(SQLException) {
        LOG_INFO("Database GetAllOfflineMessage Failed: SQLException=%s", Connection_getLastError(conn));
    }
    END_TRY;
}

/**
 *   IsExistUser:用户是否存在，表user_info
 */
bool Database::IsExistUser(Connection_T conn, const uint32_t &user_id) {
    bool res = true;
    char sql[1024] = {0};
    sprintf(sql, "SELECT * FROM user_info WHERE id = %d", user_id);
    TRY{
        ResultSet_T r = Connection_executeQuery(conn, sql);
        if (ResultSet_next(r)) {
            res = true;
        }
        else {
            res = false;
        }
    }
    CATCH(SQLException) {
        LOG_INFO("Database IsExistUser Failed: user_id=%u||SQLException=%s", user_id, Connection_getLastError(conn));
        res = false;
    }
    END_TRY;
    return res;
}

bool Database::GetOfflineMessage(Connection_T conn, const uint32_t &from_id, const uint32_t &to_id, const uint16_t &frame_id, MessageItem &msg, time_t &recv_time) {
    char sql[2048] = {0};
    sprintf(sql, "SELECT to_user, from_user, frame_id, type, text, recv_time  FROM message_cache WHERE from_user = %d AND to_user = %d AND frame_id = %d",
            from_id, to_id, frame_id);
    TRY{
        ResultSet_T r = Connection_executeQuery(conn, sql);
        if (ResultSet_next(r)) {
            msg.to_id = ResultSet_getInt(r, 1);
            msg.from_id = ResultSet_getInt(r, 2);
            msg.frame_id = ResultSet_getInt(r, 3);
            msg.type = ResultSet_getInt(r, 4);
            msg.retain = 0;
            msg.content = std::string(ResultSet_getString(r, 5));
            recv_time = ResultSet_getTimestamp(r, 6);
        }
    }
    CATCH(SQLException) {
        LOG_INFO("Database GetOfflineMessage Failed: from_id=%u||to_id=%u||fram_id=%u", from_id, to_id, frame_id);
        return false;
    }
    END_TRY;
    return true;
}
