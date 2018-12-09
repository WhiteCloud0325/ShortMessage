#include "tools/database.h"
#include "tools/logger.h"
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

/*
 function：通过电话号码查询用户
 param: 
    @number, string 电话号码
 return:
    >0 user_id
    -1 未查找到
*/
int64_t Database::GetUser(Connection_T conn, const std::string& number) {
    PreparedStatement_T p = Connection_prepareStatement(conn, "SELECT user_id FROM t_user WHERE number = ?");
    PreparedStatement_setString(p, 1, number.c_str());
    ResultSet_T r = PreparedStatement_executeQuery(p);
    int64_t user_id = -1;
    if (ResultSet_next(r)) {
        user_id = ResultSet_getInt(r, 1);
        LOG_INFO("Database GetUser: user_id=%lld||number=%s", user_id, number.c_str());
    }
    else {
        LOG_INFO("Database GetUser Select Exception: number=%s", number.c_str());
    }
    return user_id;
}

/*
    function: 添加用户
    params:
        @number: string, 电话号码
        @nickname: string, 用户昵称
        @password: string, 密码
        @ip, string 用户ip
        @user_id, int64, 返回添加用户的id
    return: 0 成功, 1 失败
*/
int Database::AddUser(Connection_T conn, const std::string &number, const std::string &nickname, const std::string &password, const std::string& ip, int64_t &user_id) {
    int res_code = REGISTER_SUCCESS;
    TRY {
        PreparedStatement_T pi = Connection_prepareStatement(conn, "INSERT INTO t_user(number, nickname, password, ip) VALUES(?, ?, ?, ?)");
        PreparedStatement_setString(pi, 1, number.c_str());
        PreparedStatement_setString(pi, 2, nickname.c_str());
        PreparedStatement_setString(pi, 3, password.c_str());
        PreparedStatement_setString(pi, 4, ip.c_str());
        PreparedStatement_execute(pi);
    }
    CATCH(SQLException) {
        LOG_INFO("Database AddUser Insert Exception: number=%s||SQLException=%s", number.c_str(), Connection_getLastError(conn));
        res_code = REGISTER_ERROR;
    }
    END_TRY;
    if (res_code !=0 ) {
        return res_code;
    }
    TRY {
        PreparedStatement_T ps = Connection_prepareStatement(conn, "SELECT user_id FROM t_user WHERE number = ?");
        PreparedStatement_setString(ps, 1, number.c_str());
        ResultSet_T r = PreparedStatement_executeQuery(ps);
        if (ResultSet_next(r)) {
            user_id = ResultSet_getInt(r, 1);
            LOG_INFO("Database AddUser: user_id=%lld||number=%s||nickname=%s||password=%s||ip=%s", user_id, number.c_str(), nickname.c_str(), password.c_str(), ip.c_str());
        }
        else {
            LOG_INFO("Database AddUser Select no result: number=%s", number.c_str());
            res_code = REGISTER_ERROR;
        }
    }
    CATCH(SQLException) {
            res_code = REGISTER_ERROR;
            LOG_INFO("Database ADDUser Select Exception: number=%s||SQLException=%s", number.c_str(),Connection_getLastError(conn));
    }
    END_TRY;
    return res_code;
}
/*
 AuthUser
    function：用户认证, 更新在线状态和ip
    params:
        @conn, 数据库连接
        @number, 用户电话号码
        @password, 用户密码
        @ip, 用户ip
        @user_id, 返回用户id
    return：
        状态码： 0 成功， 1 异常错误， 2 用户或密码错误
*/
int Database::AuthUser(Connection_T conn, const std::string &number, const std::string& password, const std::string&ip, int64_t &user_id) {
    int res_code = AUTH_SUCCESS;
    TRY {
        PreparedStatement_T ps = Connection_prepareStatement(conn, "SELECT user_id, password FROM t_user WHERE number = ?");
        PreparedStatement_setString(ps, 1, number.c_str());
        ResultSet_T r = PreparedStatement_executeQuery(ps);
        if (ResultSet_next(r)) {
            user_id = ResultSet_getInt(r, 1);
            std::string cur_password = std::string(ResultSet_getString(r, 2));
            if (cur_password == password) {
                PreparedStatement_T pi = Connection_prepareStatement(conn, "UPDATE t_user SET state = 1, SET ip = ?  WHERE number = ?");
                PreparedStatement_setString(pi, 1, ip.c_str());
                PreparedStatement_setString(pi, 2, number.c_str());
                PreparedStatement_execute(pi);
                LOG_INFO("Database AuthUser Success: user_id=%lld||number=%s||password=%s", user_id, number.c_str(), password.c_str());
            }
            else {
                res_code = AUTH_NAME_OR_PASSWORD;
                LOG_INFO("Database AuthUser Password Error: user_id=%lld||number=%s||password=%s||dbpassword=%s", user_id, number.c_str(), password.c_str(), cur_password.c_str());
            }
        }
        else {
            res_code = AUTH_NAME_OR_PASSWORD;
            LOG_INFO("Database AuthUser Number Error: number=%s", number.c_str());
        }
    }
    CATCH(SQLException) {
        res_code = AUTH_ERROR;
        LOG_INFO("Database AuthUser SQL Exception: number=%s||password=%s||ip=%s||SQLException=%s", number.c_str(), password.c_str(), ip.c_str(), Connection_getLastError(conn));
    }
    END_TRY;
    return res_code;
}

/*
    function: 用户登出
    params:
        @conn
        @user_id: 用于id查询
    return: void
*/
void LogoutUser(Connection_T conn, const int64_t &user_id) {
    TRY{
        PreparedStatement_T p = Connection_prepareStatement(conn, "Update t_user SET state = 0 WHERE user_id = ?");
        PreparedStatement_setLLong(p, 1, (long long)user_id);
        PreparedStatement_execute(p);
        LOG_INFO("Database LogoutUser Success: user_id=%lld", user_id);
    }
    CATCH(SQLException) {
        LOG_INFO("Database LogoutUser Failed: SQLException=%s", Connection_getLastError(conn));
    }
    END_TRY;
}

/*
    function: 添加好友
    params:
        @conn
        @user_id, int64_t
        @friend_id, int64_T
    return: bool 成功true , 失败 false
*/

bool Database::AddFriend(Connection_T conn, const int64_t &user_id, const int64_t &friend_id) {
    TRY {
        PreparedStatement_T p = Connection_prepareStatement(conn, "INSERT INTO t_friend(user_id, friend_id) VALUES(?, ?)");
        PreparedStatement_setLLong(p, 1, (long long) user_id);
        PreparedStatement_setLLong(p, 2, (long long) friend_id);
        PreparedStatement_execute(p);
        LOG_INFO("Database AddFriend Success: user_id=%lld||friend_id=%lld", user_id, friend_id);
        return true;
    }
    CATCH(SQLException) {
        LOG_INFO("Database AddFriend Failed: user_id=%lld||friend_id=%lld||SQLException=%s", user_id, friend_id, Connection_getLastError(conn));
        return false;
    }
    END_TRY;
}

/*
    function: 查询好友
    params:
        @conn:
        @user_id
        @friend_id
    return:
        int, 0 成功， 1 失败， 2 好友不存在
*/
int Database::QueryFriend(Connection_T conn, const int64_t &user_id, const int64_t &friend_id) {
    int res_code = QUERYFRIEND_SUCCESS;
    TRY {
        PreparedStatement_T p = Connection_prepareStatement(conn, "SELECT * FROM t_friend where user_id = ? and friend_id = ?");
        PreparedStatement_setLLong(p, 1, (long long) user_id);
        PreparedStatement_setLLong(p, 2, (long long)friend_id);
        ResultSet_T r = PreparedStatement_executeQuery(p);
        if (! ResultSet_next(r)) {
            res_code = QUERYFRIEND_NOEXIST;
        }
    }
    CATCH(SQLException) {
        res_code = QUERYFRIEND_ERROR;
        LOG_INFO("Database QueryFriend Failed: user_id=%lld||friend_id=%lld||SQLException=%s", user_id, friend_id, Connection_getLastError(conn));
    }
    END_TRY;
    return res_code;
}


