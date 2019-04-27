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
        LOG_ERROR("Database init config exception");
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
        LOG_ERROR("Database init ConnectionPool start exception");
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
    std::string get_sql = "SELECT ";
    for (int i = 1; i <= MAX_BEAM_NUM; ++i) {
        sql += "SNR_" + std::to_string(i) + " = " + std::to_string(temp_snr[i - 1]) + ",";
        get_sql += "SNR_" + std::to_string(i) + ",";
    }
    sql.pop_back();
    sql += " WHERE user_id = " + std::to_string(user_id);
    get_sql.pop_back();
    get_sql += " FROM sate_cover WHERE user_id = " + std::to_string(user_id);
    TRY {
        Connection_beginTransaction(conn);
        ResultSet_T r = Connection_executeQuery(conn, get_sql.c_str());
        if (ResultSet_next(r)) {
            std::string update_sql = "UPDATE group_cover SET ";
            for(int i = 1; i <= MAX_BEAM_NUM; ++i) {
                double  snr = ResultSet_getDouble(r, i);
                std::string tmp = "SNR_" + std::to_string(i);
                if(temp_snr[i - 1] > 0 && snr < 0) {
                    update_sql += tmp + "=" + tmp + "+1,";
                }
                else if (temp_snr[i - 1] < 0 && snr > 0) {
                    update_sql += tmp + "=if(" + tmp + ">0," + tmp + "-1,0),";
                }
            }
            update_sql.pop_back();
            if (update_sql.size() > 23) {
                update_sql += " WHERE group_id IN (SELECT gid FROM group_members WHERE uid = " + std::to_string(user_id) + ")";
                Connection_execute(conn, update_sql.c_str());
            }
        }
        Connection_execute(conn, sql.c_str());
        Connection_commit(conn);
    }
    CATCH(SQLException) {
        Connection_rollback(conn);
        LOG_ERROR("Database UpdateSateCover Failed: userid=%lld||SQLException=%s", user_id, Connection_getLastError(conn));
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


void Database::GetSateCover(Connection_T conn, const int32_t &user_id, std::vector<SateParam> &res) {
    std::string sql = "SELECT SNR_1, SNR_2, SNR_3, SNR_4, SNR_5, SNR_6, SNR_7, SNR_8, SNR_9, SNR_10	 FROM sate_cover WHERE user_id = ";
    sql += std::to_string(user_id);
    TRY {
        ResultSet_T r = Connection_executeQuery(conn, sql.c_str());
        if (ResultSet_next(r)) {
             for (int i = 1; i <= MAX_BEAM_NUM; ++i) {
                 double temp = ResultSet_getDouble(r, i);
                 if (temp > 0) {
                     res.emplace_back((i + 1) / 2, i, (float)temp);
                 }
             }
        }
    }
    CATCH(SQLException) {
        LOG_ERROR("Database SelectSateCover Failed: user_id=%lld||SQLException=%s", user_id, Connection_getLastError(conn));
    }
    END_TRY;
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
        LOG_ERROR("Database GetOfflineMessageNum Failed: user_id=%lld||SQLException=%s", user_id, Connection_getLastError(conn));
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
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm_recv_time);
    sprintf(sql, "INSERT INTO message_store(from_user, to_user, text, recv_time, send_time, frame_id, type) VALUES(%d, %d, '%s', '%s', '%s', %d, %d)",
            from_id, to_id, control_head->content, timestamp, timestamp, frame_id, control_head->type);
    TRY{
        Connection_execute(conn, sql);
        //id = Connection_lastRowId(conn);
    }
    CATCH(SQLException) {
        LOG_ERROR("Database InsertStoreMessage Failed: from_id=%u||to_id=%u||frame_id=%u||SQLException=%s", from_id, to_id, frame_id, Connection_getLastError(conn));
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
        LOG_ERROR("Database InsertStoreMessage Failed: from_id=%u||to_id=%u||frame_id=%u||SQLException=%s", msg.from_id, msg.to_id, msg.frame_id, Connection_getLastError(conn));
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
        LOG_ERROR("Database InsertOfflineMessage Failed: from_id=%u||to_id=%u||frame_id=%u||SQLException=%s", from_id, to_id, frame_id, Connection_getLastError(conn));
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
    sprintf(sql, "DELETE FROM message_cache WHERE from_user = %d AND to_user = %d AND frame_id = %d",
            from_id, to_id, frame_id);
    TRY{
        Connection_execute(conn, sql);
    }
    CATCH(SQLException) {
        LOG_ERROR("Database DeleteOfflineMessage Failed: from_id=%u||to_id=%u||frame_id=%u||SQLException=%s", from_id, to_id, frame_id, Connection_getLastError(conn));
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
        LOG_ERROR("Database UpdateOfflineMessage Failed: from_id=%u||to_id=%u||frame_id=%u||SQLException=%s", from_id, to_id, frame_id, Connection_getLastError(conn));
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
        LOG_ERROR("Database GetAllOfflineMessage Failed: SQLException=%s", Connection_getLastError(conn));
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
        LOG_ERROR("Database IsExistUser Failed: user_id=%u||SQLException=%s", user_id, Connection_getLastError(conn));
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
        LOG_ERROR("Database GetOfflineMessage Failed: from_id=%u||to_id=%u||fram_id=%u", from_id, to_id, frame_id);
        return false;
    }
    END_TRY;
    return true;
}

/**
 * function: FriendAdd
 * desc: add a friend of user
 * params:
 *      @conn,
 *      @user_id,
 *      @friend_id,
 * return: bool, true(success), false(error)
*/
bool Database::FriendAdd(Connection_T conn, const uint32_t &user_id, const uint32_t &friend_id) {
    bool res = true;
    char sql1[1024] = {0};
    char sql2[1024] = {0};

    sprintf(sql1, 
            "INSERT INTO relationship(user_1, user_2)  SELECT %d, %d  WHERE NOT EXISTS (SELECT * from relationship WHERE user_1 = %d and user_2 = %d)", 
            user_id, friend_id, user_id, friend_id);
    sprintf(sql2, 
            "INSERT INTO relationship(user_1, user_2)  SELECT %d, %d  WHERE NOT EXISTS (SELECT * from relationship WHERE user_1 = %d and user_2 = %d)", 
            friend_id, user_id, friend_id, user_id);
 
    TRY {
        Connection_beginTransaction(conn);
        Connection_execute(conn, sql1);
        Connection_execute(conn, sql2);
        Connection_commit(conn);
    }
    CATCH(SQLException) {
        res = false;
        Connection_rollback(conn);
        LOG_ERROR("Database FriendAdd Falied: SQLException=%s", Connection_getLastError(conn));
    }
    END_TRY;
    return res;
}

/**
 *  function: FriendDelete
 *  params: 
 *      @conn, Connection_T
 *      @user_id,
 *      @friend_id
 *  return: bool
 */
bool Database::FriendDelete(Connection_T conn, const uint32_t &user_id, const uint32_t &friend_id) {
    bool res = true;
    char sql1[1024] = {0};
    char sql2[1024] = {0};
    sprintf(sql1, "DELETE FROM relationship where user_1 = %d AND user_2 = %d", user_id, friend_id);
    sprintf(sql2, "DELETE FROM relationship where user_1 = %d AND user_2 = %d", friend_id, user_id);

    TRY {
        Connection_beginTransaction(conn);
        Connection_execute(conn, sql1);
        Connection_execute(conn, sql2);
        Connection_commit(conn);
    }
    CATCH(SQLException) {
        res = false;
        Connection_rollback(conn);
        LOG_ERROR("Database FriendDelete Failed: SQLException=%s", Connection_getLastError(conn));
    }
    END_TRY;
    return res;
}

/**
 *  function: FriendList
 *  params:
 *      @conn,
 *      @user_id,
 *      @friends, return value
 *  return: bool, success true, exception false;
 */
bool Database::FriendList(Connection_T conn, const uint32_t &user_id, std::vector<UserInfo> &friends) {
    bool res = true;
    char sql[1024] = {0};
    sprintf(sql, 
            "SELECT user_info.id, user_info.name FROM user_info, relationship WHERE relationship.user_1 = %d AND user_info.id = relationship.user_2",
            user_id);
    TRY {
        ResultSet_T r = Connection_executeQuery(conn, sql);
        while(ResultSet_next(r)) {
            friends.emplace_back((uint32_t)ResultSet_getInt(r, 1), ResultSet_getString(r, 2));
        }
    }
    CATCH(SQLException) {
        res = false;
        LOG_ERROR("Database FriendList Falied: user_id=%d||SQLException=%s", user_id, Connection_getLastError(conn));
    }
    END_TRY;
    return res;
    
}

/*  function: GroupCreate
    params:
        @conn, Connecton_T
        @user_id, uint32_t
        @group_name, string
        @members
    return:
        gourp_id, int
*/
int Database::GroupCreate(Connection_T conn, const uint32_t &user_id, const std::string &group_name) {
    char create_sql[128] = {0};
    int group_id = -1;
    sprintf(create_sql, "INSERT INTO group_info SET group_name = \"%s\"",group_name.c_str());
    TRY {
        Connection_beginTransaction(conn);
        Connection_execute(conn, create_sql);
        group_id = Connection_lastRowId(conn);
        char group_cover[128] = {0};
        snprintf(group_cover, 128, "INSERT INTO group_cover SET group_id = %d", group_id);
        Connection_execute(conn, group_cover.c_str());
        Connection_commit(conn);
    }
    CATCH(SQLException) {
        group_id = -1;
        Connection_rollback(conn);
        LOG_ERROR("Database CreateGroup Failed: SQLException=%s", Connection_getLastError(conn));
    }
    END_TRY;
    return group_id;
}

/*
    function: GroupAddMember
    desc: add a group of members;
    params:
        @conn,
        @gourp_id,
        @members
*/
bool Database::GroupAddMembers(Connection_T conn, const uint32_t &group_id, const  std::vector<uint32_t> &members) {
    bool res = true;
    if (members.empty()) {
        LOG_ERROR("Database GroupAddMemeber Failed: members empty");
        return false;
    }
    std::string add_members = "INSERT INTO group_members(gid, uid) VALUES";
    for (int i = 0; i < members.size(); ++i) {
        add_members += "(" + std::to_string(group_id) + "," + std::to_string(members[i]) + "),";
    }
    add_members.pop_back();
    std::string sate_cover = "SELECT ";
    for (int i = 1; i <= MAX_BEAM_NUM; ++i) {
        sate_cover += "SUM(SNR_";
        sate_cover += std::to_string(i);
        sate_cover += ">0),"; 
    }
    sate_cover.pop_back();
    sate_cover += " FROM sate_cover WHERE user_id IN (";
    for (auto id: members) {
        sate_cover += std::to_string(id);
        sate_cover += ",";
    }
    sate_cover[sate_cover.size() - 1] = ')';
    TRY {
        Connection_beginTransaction(conn);
        Connection_execute(conn, add_members.c_str());
        ResultSet_T r = Connection_executeQuery(conn, sate_cover.c_str());
        if (ResultSet_next(r)) {
            std::string group_cover = "UPDATE group_cover SET ";
            for (int i = 1; i <= MAX_BEAM_NUM; ++i) {
                int num = ResultSet_getInt(r, i);
                group_cover += "SNR_" + std::to_string(i) + "=SNR_" + std::to_string(i) + "+" + std::to_string(num) + ",";
            }
            group_cover[group_cover.size() - 1] = ' ';
            group_cover += "WHERE group_id=" + std::to_string(group_id);
            Connection_execute(conn, group_cover.c_str());
        }
        Connection_commit(conn);
    }
    CATCH(SQLException) {
        Connection_rollback(conn);
        res = false;
        LOG_ERROR("Database GroupAddMember Failed: SQLException=%s", Connection_getLastError(conn));
    }
    END_TRY;
    return res;
}

/**
    function: GroupDeleteMember
    desc: delete a member from group
    params:
        @conn,
        @group_id,
        @member
    return: bool
*/
/*bool Database::GroupDeleteMember(Connection_T conn, const uint32_t &group_id, const uint32_t &member) {
    bool res = true;
    char sql[256] = {0};
    sprintf(sql, "DELETE FROM group_members WHERE gid = %d AND uid = %d", (int)group_id, (int)member);
    TRY {
        Connection_execute(conn, sql);
    }
    CATCH(SQLException) {
        res = false;
        LOG_ERROR("Database GroupDeleteMember Failed: SQLException=%s", Connection_getLastError(conn));
    }
    END_TRY;
    return res;
}*/

/**
 *  function: GroupDeleteMembers
 *  desc: delete membres from group
 *  params: 
 *      @conn
 *      @group_id
 *      @members
 *  return: 
 */ 
bool Database::GroupDeleteMembers(Connection_T conn, const uint32_t &group_id, const std::vector<uint32_t> &members) {
    bool res = true;
    std::string sql = "DELETE FROM group_members WHERE gid = ";
    sql += std::to_string(group_id) + " AND uid IN(";
    for (uint32_t i = 0; i < members.size(); ++i) {
        sql += std::to_string(members[i]) + ",";
    }
    sql.pop_back();
    sql.push_back(')');
    std::string sate_cover = "SELECT ";
    for (int i = 1; i <= MAX_BEAM_NUM; ++i) {
        sate_cover += "SUM(SNR_";
        sate_cover += std::to_string(i);
        sate_cover += ">0),"; 
    }
    sate_cover.pop_back();
    sate_cover += " FROM sate_cover WHERE user_id IN (";
    for (auto id: members) {
        sate_cover += std::to_string(id);
        sate_cover += ",";
    }
    sate_cover[sate_cover.size() - 1] = ')';
    TRY {
        Connection_beginTransaction(conn);
        // delete members from group_members
        Connection_execute(conn, sql.c_str());
        // get sate_cover of members;
        ResultSet_T r = Connection_executeQuery(conn, sate_cover.c_str());
        if (ResultSet_next(r)) {
            std::string group_cover = "UPDATE group_cover SET ";
            for (int i = 1; i <= MAX_BEAM_NUM; ++i) {
                std::string id = std::to_string(i);
                std::string num = std::to_string(ResultSet_getInt(r, i));
                group_cover += "SNR_" + id + "=if(SNR_" + id + ">" + num + ", SNR_" + id + "-" + num + ", 0),";
            }
            group_cover[group_cover.size() - 1] = ' ';
            group_cover += "WHERE group_id=" + std::to_string(group_id);
            Connection_execute(conn, group_cover.c_str());
        }
        Connection_commit(conn);
    }
    CATCH(SQLException) {
        Connection_rollback(conn);
        LOG_ERROR("Database GroupDeleteMembers Failed: gid=%d||SQLException=%s", group_id, Connection_getLastError(conn));
        return false;
    }
    END_TRY;
    return res;
}

/**
 *  function: GroupGetSateCover
 *  desc: get the beam id of Group Cover
 *  params:
 *      @conn,
 *      @group_id,
 *      @res, return value, std::vector<int>
 *  return: void
 */
void Database::GroupGetSateCover(Connection_T conn, const uint32_t &group_id, std::vector<int> &res) {
    std::string sql = "SELECT ";
    for (int i = 1; i <= MAX_BEAM_NUM; ++i) {
        sql += "SNR_" + std::to_string(i) + ",";
    }
    sql[sql.size() - 1] = ' ';
    sql += " FROM group_cover WHERE group_id = " + std::to_string(group_id);
    TRY{
        ResultSet_T r = Connection_executeQuery(conn, sql.c_str());
        if (ResultSet_next(r)) {
            for (int i = 1; i <= MAX_BEAM_NUM; ++i) {
                int num = ResultSet_getInt(r, i);
                if (num > 0) {
                    res.emplace_back(i);
                }
            }
        }
    }
    CATCH(SQLException) {
        LOG_ERROR("Database GroupGetSateCover Failed: gid=%d", group_id);
    }
    END_TRY;
    return;
}


/**
    function: GroupListByUserId
    desc: list all gourps of the user
    params:
        @conn,
        @user_id,
        @groups, return value
    return: bool
*/
bool Database::GroupListByUserId(Connection_T conn, const uint32_t &user_id, std::vector<GroupInfo> &groups) {
    bool res = true;
    char sql[256] = {0};
    sprintf(sql, "SELECT group_info.gid, group_info.group_name FROM group_info, group_members WHERE group_info.gid = group_members.gid AND group_members.uid = %d", user_id);
    TRY {
        ResultSet_T r = Connection_executeQuery(conn, sql);
        while(ResultSet_next(r)){
            groups.emplace_back((uint32_t)ResultSet_getInt(r, 1), ResultSet_getString(r, 2));
        }
    }
    CATCH(SQLException){
        res = false;
        LOG_ERROR("Database GroupListByUserId Failed: SQLException=%s", Connection_getLastError(conn));
    }
    END_TRY;
    return res;
}

/**
    function: GroupListUser
    desc:list users of the group
    params:
        @conn,
        @group_id,
        @user_infos, return value
    return: bool
*/
bool Database::GroupListUser(Connection_T conn, const uint32_t &group_id, std::vector<UserInfo> &user_infos) {
    bool res = true;
    char sql[256] = {0};
    sprintf(sql,"SELECT user_info.id, user_info.name FROM user_info, group_members WHERE group_members.gid = %d AND group_members.uid = user_info.id", group_id);
    TRY {
        ResultSet_T r = Connection_executeQuery(conn, sql);
        while(ResultSet_next(r)) {
            user_infos.emplace_back(ResultSet_getInt(r, 1), ResultSet_getString(r, 2));
        }
    }
    CATCH(SQLException) {
        res = false;
        LOG_ERROR("Database GroupListUser Failed: SQLException=%s", Connection_getLastError(conn));
    }
    END_TRY;
    return res;
}

/**
 *  function: GroupListUserId
 *  desc: list all user ids of the group
 *  params:
 *      @conn,
 *      @group_id
 *  return: std::vector<uint32_t> 
 */ 
void Database::GroupListUserId(Connection_T conn, const uint32_t &group_id, std::vector<uint32_t> &res) {
    char sql[256] = {0};
    snprintf(sql, 256, "SELECT uid FROM group_members WHERE gid = %d", group_id);
    TRY {
        ResultSet_T r = Connection_executeQuery(conn, sql);
        while(ResultSet_next(r)) {
            res.push_back((uint32_t) ResultSet_getInt(r, 1));
        }
    }
    CATCH(SQLException) {
        LOG_ERROR("Database GroupListUserId Failed: gid=%d||SQLException=%s", group_id, Connection_getLastError(conn));
    }
    END_TRY;
}

/**
 *  function: GroupMessageInsert
 *  desc: Insert message into group_msgs
 *  params:
 *      @conn,
 *      @group_id
 *      @user_id,
 *      @content
 *  return: int64_t, msg_id, success >0 ,else -1
 */
int64_t Database::GroupMessageInsert(Connection_T conn, const uint32_t &group_id, const uint32_t &user_id, const char* content, const time_t &recv_time) {
    int64_t msg_id = -1;
    char sql[4096] = {0};
    tm tm_recv_time;
    gmtime_r(&recv_time, &tm_recv_time);
    char timestamp[64] = {0};
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm_recv_time);
    snprintf(sql, 4096, "INSERT INTO group_msgs(gid, sender_uid, time, content) VALUES(%d, %d, '%s', '%s')", group_id, user_id, timestamp, content);
    TRY {
        Connection_execute(conn, sql);
        msg_id = Connection_lastRowId(conn);
    }
    CATCH(SQLException) {
        msg_id = -1;
        LOG_ERROR("Database GroupMessageInsert Failed: group_id=%d||sender_uid=%d||content=%s||SQLException=%s", group_id, user_id, content, Connection_getLastError(conn));
    }
    END_TRY;
    return msg_id;
}

/**
 *  function: GroupMessageUpdate
 *  desc: update group_members last_ack_msg_id
 *  params:
 *      @conn
 *      @group_id
 *      @user_id
 *      @msg_id
 *  return: bool 
 */
bool Database::GroupMessageUpdate(Connection_T conn, const uint32_t &group_id, const uint32_t &user_id, const uint64_t &msg_id) {
    bool res = true;
    char sql[1024] = {0};
    snprintf(sql, 1024, "UPDATE group_members SET last_ack_msgid = %d where gid = %d AND  uid = %d AND last_ack_msgid < %lld", msg_id, group_id, user_id, msg_id);
    TRY {
        Connection_execute(conn, sql);
    }
    CATCH(SQLException) {
        res = false;
        LOG_ERROR("Database GroupMessageUpdate Failed: gid=%d||uid=%d||msg_id=%lld||SQLException=%s", group_id, user_id, msg_id, Connection_getLastError(conn));
    }
    END_TRY;
    return res;
}

/**
 *  function GroupMessagePull
 *  desc: the user pull offline groupmessages 
 *  params: 
 *      @conn
 *      @group_id
 *      @user_id
 *      @messages, return value
 *  return: int, message num, success >=0, error -1
 */
int Database::GroupMessagePull(Connection_T conn, const uint32_t &group_id, const uint32_t &user_id, std::vector<GroupMessage> &messages) {
    int num = 0;
    char sql[2048] = {0};
    snprintf(sql, 2048, "SELECT content, time FROM group_msgs WHERE group_msgs.gid = %d AND msgid > (SELECT last_ack_msgid FROM group_members WHERE uid = %d)", group_id, user_id);
    TRY {
        ResultSet_T r = Connection_executeQuery(conn, sql);
        while (ResultSet_next(r)) {
            messages.emplace_back(ResultSet_getString(r, 1), ResultSet_getTimestamp(r, 2));
        }
    }
    CATCH(SQLException) {
        num = -1;
        LOG_ERROR("Database GroupMessagePull Failed: gid=%d||uid=%d||SQLEXception=%s", group_id, user_id, Connection_getLastError(conn));
    }
    END_TRY;
    return messages.size();
}

/**
 *  function IsUserInGroup
 *  desc:  
 *  params:
 *      @conn,
 *      @group_id,
 *      @user_id
 *  return, int
 */
int  Database::IsUserInGroup(Connection_T conn, const uint32_t &group_id, const uint32_t &user_id) {
    char sql[256] = {0};
    snprintf(sql, 256, "SELECT * FROM group_members WHERE gid = %d AND uid = %d", group_id, user_id);
    TRY {
        ResultSet_T r = Connection_executeQuery(conn, sql);
        if (ResultSet_next(r)) {
            return 1;
        }
        else {
            return 0;
        }
    }
    CATCH(SQLException) {
        LOG_ERROR("Database IsUserInGroup Failed: gid=%d||uid=%d||SQLException=%s", group_id, user_id, Connection_getLastError(conn));
        return -1;
    }
    END_TRY;
 }
