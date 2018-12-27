#ifndef send_helper_h_
#define send_helper_h_

#include <string>
#include "thrift/LogicInterface.h"

class SendHelper {
public:
    ~SendHelper(){}
    static SendHelper* GetInstance() {
        if (instance_ == NULL) {
            instance_ = new SendHelper();
        }
        return instance_;
    }
    bool Init(const std::string &ip, const int port);
    void SendMessage(const std::string &buf);
private:
    SendHelper():logic_port_(0){}
    static SendHelper* instance_;
private:
    int logic_port_;
    std::string logic_ip_;
};


#endif /*define send_helper_h_*/