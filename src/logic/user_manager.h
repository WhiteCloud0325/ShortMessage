#ifndef USER_MANAGER_H_
#define USER_MANAGER_H_

#include <string>
#include "tools/database.h"


class UserManager  {
public:
    UserManager():database_(NULL){}
    ~UserManager();
    bool Init(Database *database);
    void UserRegister(const std::string &request, int32_t &type, int64_t uid, std::string &response, std::string &ip);
    void UserAuth(const std::string request, int32_t &type, int64_t &uid, std::string &response, std::string &ip);
    void UserLogout(std::string request);

private:
    bool UserRegisterRequestDecode(const std::string &request, std::string &number, std::string &password, std::string &nickname, std::string& ip);
    std::string UserRegisterResponseEncode(const int &status, const char * err_msg);
    bool UserAuthRequestDecode(const std::string &request, std::string &number, std::string &password, std::string &ip);
    std::string UserAuthResponseEncode(const int &status, const int64_t &uid, const char *err_msg);
private:
    Database *database_;
};

#endif /* define USER_MANAGER_H_*/
