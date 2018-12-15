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
    void SendMessageData(Protocol &p);
    void SendData();
private:
    SendHelper():access_port_(0){}
    static SendHelper* instance_;
private:
    int access_port_;
    std::string access_ip_;
};


#endif /*define send_helper_h_*/