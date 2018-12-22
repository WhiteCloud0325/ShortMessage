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
        temp_snr[sate_cover.beam_id[i] - 1] = sate_cover.snr[i];
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


std::vector<int32_t> Database::GetSateCover(Connection_T conn, const int32_t &user_id) {
    std::vector<int32_t> res; 
    TRY {
        PreparedStatement_T p = Connection_prepareStatement(conn, "SELECT SNR_1, SNR_2, SNR_3, SNR_4, SNR_5, SNR_6, SNR_7, SNR_8, SNR_9, SNR_10	 FROM sate_cover WHERE user_id = ?");
        PreparedStatement_setInt(p, 1, (int)user_id);
        ResultSet_T r = PreparedStatement_executeQuery(p);
        if (ResultSet_next(r)) {
             for (int i = 1; i <= MAX_BEAM_NUM; ++i) {
                 double temp = ResultSet_getDouble(r, i);
                 if (temp > 0) {
                     res.push_back(i);
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
    TRY {
        PreparedStatement_T p = Connection_prepareStatement(conn, "SELECT COUNT(*) FROM message_cache WHERE to_user = ?");
        PreparedStatement_setInt(p, 1, (int)user_id);
        ResultSet_T r = PreparedStatement_executeQuery(p);
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
 *  return: int 插入的id
 */ 
int64_t Database::InsertStoreMessage(Connection_T conn, ControlHead* control_head) {
    int64_t id = -1;
    if (control_head == NULL) {
        return id;
    }
    uint32_t to_id = ntohl(control_head->to_id);
    uint32_t from_id = ntohl(control_head->from_id);
    uint16_t frame_id = ntohs(control_head->frame_id);
    time_t timestamp = time(NULL);
    TRY{
        PreparedStatement_T p = Connection_prepareStatement(conn, "INSERT INTO message_store(from_user, to_user, text, recv_time, frame_id, type) VALUES(?, ?, ?, ?, ?, ?)");
        PreparedStatement_setInt(p, 1, (int)from_id);
        PreparedStatement_setInt(p, 2, (int)to_id);
        PreparedStatement_setString(p, 3, control_head->content);
        PreparedStatement_setTimestamp(p, 4, timestamp);
        PreparedStatement_setInt(p, 5, (int)frame_id);
        PreparedStatement_setInt(p, 6, (int) control_head->type);
        PreparedStatement_execute(p);
        id = Connection_lastRowId(conn);
    }
    CATCH(SQLException) {
        LOG_INFO("Database InsertStoreMessage Failed: from_id=%u||to_id=%u||frame_id=%u||SQLException=%s", from_id, to_id, frame_id, Connection_getLastError(conn));
        return id;
    }
    END_TRY;
    return id;
}

/** function:InsertOfflineMessage 消息缓存
 *  params：
 *         @conn, Connetion)T
 *         @control_head, ControlHead
 *  return: bool
 */ 
bool Database::InsertOfflineMessage(Connection_T conn, ControlHead* control_head, const int64_t& id) {
    if (control_head == NULL) {
        return false;
    }
    uint32_t to_id = ntohl(control_head->to_id);
    uint32_t from_id = ntohl(control_head->from_id);
    uint16_t frame_id = ntohs(control_head->frame_id);
    time_t timestamp = time(NULL);
    TRY{
        PreparedStatement_T p = Connection_prepareStatement(conn, "INSERT INTO message_cache(message_id, from_user, to_user, type, text, recv_time, frame_id, retry_time) VALUES(?, ?, ?, ?, ?, ?, ?, ?)");
        PreparedStatement_setLLong(p, 1, (long long)id);
        PreparedStatement_setInt(p, 2, (int)from_id);
        PreparedStatement_setInt(p, 3, (int)to_id);
        PreparedStatement_setInt(p, 4, (int) control_head->type);
        PreparedStatement_setString(p, 5,  control_head->content);
        PreparedStatement_setTimestamp(p, 6, timestamp);
        PreparedStatement_setInt(p, 7, (int)frame_id);
        PreparedStatement_setTimestamp(p, 8, timestamp + 10);
        PreparedStatement_execute(p);
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
    TRY{
        PreparedStatement_T p = Connection_prepareStatement(conn, "DELETE FROM message_cache WHERE from_user = ? AND to_user = ? AND frame_id = ? ");
        PreparedStatement_setInt(p, 1, (int)from_id);
        PreparedStatement_setInt(p, 2, to_id);
        PreparedStatement_setInt(p, 3, frame_id);
        PreparedStatement_execute(p);
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
    TRY{
        PreparedStatement_T p = Connection_prepareStatement(conn, "UPDATE message_cache SET retry_num = ?, retry_time = ? WHERE from_user = ? AND to_user = ? AND frame_id = ?");
        PreparedStatement_setInt(p, 1, retry_num);
        PreparedStatement_setTimestamp(p, 2, timestamp);
        PreparedStatement_setInt(p, 3, (int)from_id);
        PreparedStatement_setInt(p, 4, (int)to_id);
        PreparedStatement_setInt(p, 5, (int)frame_id);
        PreparedStatement_execute(p);
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
    TRY{
        PreparedStatement_T p = Connection_prepareStatement(conn, "SELECT to_user, from_user, frame_id, type, text, retry_num FROM message_cache WHERE retry_num < ? AND retry_time <= ?");
        PreparedStatement_setInt(p, 1, max_retry_num);
        PreparedStatement_setTimestamp(p, 2, cur_timestamp);
        ResultSet_T r = PreparedStatement_executeQuery(p);
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
            PreparedStatement_T pu = Connection_prepareStatement(conn, "UPDATE message_cache SET retry_num = ?, retry_time = ? WHERE to_user = ? AND from_user = ? AND frame_id = ?");
            PreparedStatement_setInt(pu, 1, retry_num);
            PreparedStatement_setTimestamp(pu, 2, cur_timestamp + retry_num * retry_interval);
            PreparedStatement_setInt(pu, 3, (int)message.to_id);
            PreparedStatement_setInt(pu, 4, (int)message.from_id);
            PreparedStatement_setInt(pu, 5, (int)message.frame_id);
            PreparedStatement_execute(pu);
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
    TRY{
        PreparedStatement_T p = Connection_prepareStatement(conn, "SELECT * FROM user_info WHERE id = ?");
        PreparedStatement_setInt(p, 1, (int)user_id);
        ResultSet_T r = PreparedStatement_executeQuery(p);
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
