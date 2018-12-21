#ifndef send_helper_h_
#define send_helper_h_

#include <libconfig.h++>
#include <string>
#include "thrift/LogicInterface.h"
#include "logic/protocol.h"

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
    void SendMessage(const uint32_t user_id, const std::string &buf, std::vector<int32_t> &beams);
private:
    SendHelper():access_port_(0){}
    static SendHelper* instance_;
private:
    int access_port_;
    std::string access_ip_;
};


#endif /*define send_helper_h_*/