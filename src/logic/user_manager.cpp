#include "logic/user_manager.h"
#include "proto/im.pb.h"
#include "tools/logger.h"
using namespace im;

UserManager::~UserManager(){}

bool UserManager::Init(Database *database){
    database_ = database;
    return true;
}


/*
    function:注册
    param: 
        @request, string
    return:
        int, 0成功, 1异常, 2已注册
*/
void UserManager::UserRegister(const std::string &request, int32_t &type, int64_t uid, std::string &response, std::string &ip) {
    std::string number, password, nickname;
    uid = -1;
    if (!UserRegisterRequestDecode(request, number, password, nickname, ip)) {
        response = UserRegisterResponseEncode(REGISTER_ERROR, "UserRegister Parse Error");
        LOG_INFO("UserRegister Parse request error");
        return;
    }
    
    Connection_T conn = database_->GetConnection();
    if (conn == NULL) {
        response = UserRegisterResponseEncode(REGISTER_ERROR, "System Error");
        LOG_INFO("UserRegister Database GetConnection Failed");
        return;
    }

    uid = database_->GetUser(conn, number);

    if (uid >= 0) {
        response = UserRegisterResponseEncode(REGISTER_NUMBERUSED, "Number already userd");
        LOG_INFO("UserRegister Number already userd: number=%s||uid=%lld", number.c_str(), uid);
        Connection_close(conn);
        return;
    } 

    int res_code = database_-> AddUser(conn, number, nickname, password, ip, uid);

    if (res_code != 0) {
        response = UserRegisterResponseEncode(REGISTER_ERROR, "System Error");
        Connection_close(conn);
        return;
    }

    response = UserRegisterResponseEncode(REGISTER_SUCCESS, "Success");
    Connection_close(conn);
    return;
}

void UserManager::UserAuth(const std::string request, int32_t &type, int64_t &uid, std::string &response, std::string &ip) {
    std::string number, password;
    int res_code = 0;
    if (!UserAuthRequestDecode(request, number, password, ip)) {
        response = UserAuthResponseEncode(AUTH_ERROR, -1, "Request Parse Failed");
        LOG_INFO("UserAuth Parse Request failed");
        return;
    }

    Connection_T conn = database_->GetConnection();
    if (conn == NULL) {
        response = UserAuthResponseEncode(AUTH_ERROR, -1, "System Error");
        LOG_INFO("UserAuth GetConnection Failed");
        return;
    }

    res_code = database_->AuthUser(conn, number, password, ip, uid);

    switch(res_code) {
        case AUTH_SUCCESS:
            response = UserAuthResponseEncode(res_code, uid, "Auth Success");
        case AUTH_ERROR:
            response = UserAuthResponseEncode(res_code, -1, "System Error");
        case AUTH_NAME_OR_PASSWORD:
            response = UserAuthResponseEncode(res_code, -1, "Number or password error");
    }
    Connection_close(conn);
    return;


}



/*
    function:   注册解码
    params：
        @request
        @number
        @password
        @
*/
bool UserManager::UserRegisterRequestDecode(const std::string &request, std::string &number, std::string &password, std::string &nickname, std::string& ip) {
    UserRegisterRequest user_register_request;
    if (!user_register_request.ParseFromString(request)) {
        return false;
    }

    number = user_register_request.number();

    password = user_register_request.password();

    nickname = user_register_request.nickname();

    ip = user_register_request.ip();

    return true;
}

/*
    function: 用户注册response 序列化
*/
std::string UserManager::UserRegisterResponseEncode(const int &status, const char * err_msg) {
    UserRegisterResponse user_register_response;
    std::string response;
    user_register_response.set_status(status);
    user_register_response.set_err_msg(err_msg);
    user_register_response.SerializeToString(&response);
    return response;
}

/*
    function:用户登录request解码
*/
bool UserManager::UserAuthRequestDecode(const std::string &request, std::string &number, std::string &password, std::string &ip) {
    UserLoginRequest user_login_reqeust;
    if (!user_login_reqeust.ParseFromString(request)) {
        return false;
    }

    number = user_login_reqeust.number();

    password = user_login_reqeust.password();

    ip = user_login_reqeust.ip();

    return true;

}

/**
 *  function
 */
std::string UserManager::UserAuthResponseEncode(const int &status, const int64_t &uid, const char *err_msg) {
    UserLoginResponse user_login_response;
    std::string response;
    user_login_response.set_status(status);
    user_login_response.set_uid(uid);
    user_login_response.set_err_msg(err_msg);
    user_login_response.SerializeToString(&response);
    return response;
}
 
