#ifndef DATABASE_H_
#define DATABASE_H_

#include <zdb.h>
#include <libconfig.h++>
#include <stdint.h>
enum RegisterResultCode {
     REGISTER_SUCCESS = 0, //注册成功
     REGISTER_ERROR = 1,     //未知错误
     REGISTER_NUMBERUSED = 2    //手机号已使用
};
enum AuthResultCode {
     AUTH_SUCCESS = 0,   //认证成功
     AUTH_ERROR = 1,     //未知错误
     AUTH_NAME_OR_PASSWORD = 2   //用户名或密码错误
};



enum QueryFriendResultCode {
    QUERYFRIEND_SUCCESS = 0, // 查到好友
    QUERYFRIEND_ERROR = 1, // 异常错误
    QUERYFRIEND_NOEXIST = 2, //好友不存在
};

class Database {
public:
    Database(){}
    bool Init(const libconfig::Setting &setting);
    ~Database();
    Connection_T GetConnection();
    //bool Register(const std::string &number, const std::string &nickname, const std::string &password, uint64_t &user_id);
    int64_t GetUser(Connection_T conn, const std::string &number);
    int AddUser(Connection_T conn, const std::string &number, const std::string &nickname, const std::string &password, const std::string &ip, int64_t &user_id);
    int AuthUser(Connection_T conn, const std::string &number, const std::string &password, const std::string &ip, int64_t &user_id);
    void LogoutUser(Connection_T conn, const int64_t &user_id);
    bool AddFriend(Connection_T conn, const int64_t &user_id, const int64_t &friend_id);
    int QueryFriend(Connection_T conn, const int64_t &user_id, const int64_t &friend_id);
private:
    std::string host_name_;
    std::string user_;
    std::string password_;
    std::string db_name_;
    URL_T mysql_url_;
    ConnectionPool_T pool_;
};


#endif  /* define database_h*/
