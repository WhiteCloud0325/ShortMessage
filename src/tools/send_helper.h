#ifndef send_helper_h_
#define send_helper_h_

#include <libconfig.h++>
#include <string>
#include "thrift/LogicInterface.h"
#include "tools/protocol.h"

class SendHelper {
public:
    ~SendHelper(){}
    static SendHelper* GetInstance() {
        if (instance_ == NULL) {
            instance_ = new SendHelper();
        }
        return instance_;
    }
    bool Init(libconfig::Setting &setting);
    void SendMessage(const std::string &buf);
private:
    SendHelper():logic_port_(0){}
    static SendHelper* instance_;
private:
    int logic_port_;
    std::string logic_ip_;
};


#endif /*define send_helper_h_*/