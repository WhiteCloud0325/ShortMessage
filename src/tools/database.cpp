#include "tools/database.h"
#include "tools/logger.h"

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
 *          @conn
 *          @user_id
 *          @ids
 * return: bool
*/
bool Database::UpdateSateCover(Connection_T conn, const int64_t &user_id, const Satellite& sate_cover) {
    if (sate_cover.sat_cover_num == 0) {
        return true;
    } 
    std::vector<float> temp_snr(MAX_BEAM_NUM, -1);
    for (int i = 0; i < sate_cover.sat_cover_num; ++i) {
        temp_snr[sate_cover.beam_id[i]] = sate_cover.snr[i];
    } 
    std::string sql = "Update sate_cover SET ";
    for (int i = 1; i <= MAX_BEAM_NUM; ++i) {
        sql += "SNR_" + std::to_string(i) + " = " + std::to_string(temp_snr[i - 1]) + ",";
    }
    sql.pop_back();
    sql += " WHERE user_id = " + std::to_string(user_id) + ")";
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

/** UnsertNetworkCover
 *  function: 设置NetworkCover net_type 为0
 *  params:
 *          @conn
 *          @user_id
 *          @ids
 *  return: bool
*/
bool Database::UnsetSateCover(Connection_T conn, const int64_t &user_id, const std::vector<int> &ids) {
    if (ids.empty()) {
        return true;
    }
    std::string sql = "Update sate_cover SET sate_type = 0 WHERE user_id = ";
    sql += std::to_string(user_id);
    sql += " and sate_beam_id IN( ";
    for (auto i : ids) {
        sql += std::to_string(i) + ",";
    }
    sql.pop_back();
    sql.push_back(')');
    TRY {
        Connection_execute(conn, sql.c_str());
    }
    CATCH(SQLException) {
        LOG_INFO("Database UnsetSateCover Failed: userid=%lld||SQLException=%s", user_id, Connection_getLastError(conn));
        return false;
    }
    END_TRY;
    return true;
}

int Database::SelectSateCover(Connection_T conn, const int64_t &user_id) {
    int res = -1;
    TRY {
        PreparedStatement_T p = Connection_prepareStatement(conn, "SELECT sate_beam_id, SNR	FROM sate_cover WHERE user_id = ? and sate_type =1 ORDER BY SNR DESC LIMIT 1");
        PreparedStatement_setLLong(p, 1, (long long)user_id);
        ResultSet_T r = PreparedStatement_executeQuery(p);
        if (ResultSet_next(r)) {
            res = ResultSet_getInt(r, 1);
        }
    }
    CATCH(SQLException) {
        LOG_INFO("Database SelectSateCover Failed: user_id=%lld", user_id);
    }
    END_TRY;
    return res;
}

/**
 *  function: InserMessage 插入消息
 *  params:
 *      @conn
 *      @message
 *  return: int64_t, if success return insert msg_id
 *                   else return -1
 *  notes： PreparedStatement_setTimestamp是将time_t 转换为GMT时间戳
 */
int64_t Database::InsertMessage(Connection_T conn, const int64_t &from_id, const int64_t &to_id, const std::string &content, const time_t &timestamp) {
    int64_t msg_id = -1;
    TRY {
        PreparedStatement_T p = Connection_prepareStatement(conn, "INSERT INTO message_store(from_id, to_id, content, time) VALUES(?, ?, ?, ?)");
        PreparedStatement_setLLong(p, 1, (long long) from_id);
        PreparedStatement_setLLong(p, 2, (long long) to_id);
        PreparedStatement_setString(p, 3, content.c_str());
        PreparedStatement_setTimestamp(p, 4, timestamp);
        PreparedStatement_execute(p);
        msg_id = Connection_lastRowId(conn);
    }
    CATCH(SQLException) {
        LOG_INFO("Database InsertMessage Failed: from_id=%lld||to_id=%lld", from_id, to_id);
    }
    END_TRY;
    return msg_id;
}

/**
 *   function: GetOfflineMessage 获取离线消息
 *   params:
 *      @conn
 *      @
 * 
 *   notes：ResultSet_getTimestamp是采用GMT时间转换
 */
void Database::GetOfflineMessage(Connection_T conn, const int64_t &to_id, int limit_num) {
    TRY {
        PreparedStatement_T p = Connection_prepareStatement(conn, "SELECT msg_id, from_id, to_id, content, time FROM message_store WHERE to_id = ? and status = 0 LIMIT ?");
        PreparedStatement_setLLong(p, 1, (long long)to_id);
        PreparedStatement_setInt(p, 2, limit_num);
        ResultSet_T r = PreparedStatement_executeQuery(p);
        while (ResultSet_next(r)) {
            int64_t msg_id = ResultSet_getLLong(r, 1);
            int64_t from_id = ResultSet_getLLong(r, 2);
            int64_t to_id = ResultSet_getLLong(r, 3);
            std::string content = ResultSet_getString(r, 4);
            time_t timestamp = ResultSet_getTimestamp(r, 5);
        }
    }
    CATCH(SQLException) {
        LOG_INFO("Database GetOfflineMessage Failed: to_id=%lld", to_id);
    }
    END_TRY;
}

bool Database::SetStateMessage(Connection_T conn, const int64_t &msg_id) {
    TRY {
        PreparedStatement_T p = Connection_prepareStatement(conn, "Update message_store SET status = 1 WHERE msg_id = ?");
        PreparedStatement_setLLong(p, 1, (long long) msg_id);
        PreparedStatement_execute(p);
    }
    CATCH(SQLException) {
        LOG_INFO("SetStateMessage Exception: msg_id=%lld||Exception=%s", msg_id, Connection_getLastError(conn));
    }
    END_TRY;
}

bool Database::UserOnline(Connection_T conn, const int64_t &user_id) {
    TRY{
        PreparedStatement_T p = Connection_prepareStatement(conn, "SELECT state FROM t_user WHERE user_id = ?");
        PreparedStatement_setLLong(p, 1, (long long) user_id);
        ResultSet_T r = PreparedStatement_executeQuery(p);
        if (ResultSet_next(r)) {
            bool state = ResultSet_getInt(r, 1);
            if (state) {
                return true;
            }   
        }
    }
    CATCH(SQLException) {
        LOG_INFO("Database UserOnline Exception: user_id=%lld", user_id);
    }
    END_TRY;
    return false;
}
