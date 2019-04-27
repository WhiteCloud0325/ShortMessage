#ifndef send_helper_h_
#define send_helper_h_

#include <libconfig.h++>
#include <string>
#include "thrift/LogicInterface.h"
#include "logic/protocol.h"
#include <map>

struct Address{
    Address(){}
    Address(int port_, const std::string &ip_):port(port_), ip(ip_){}
    Address(const Address &address){
        this->port = address.port;
        this->ip = address.ip;
    }
    std::string ip;
    int port;
};

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
    bool SendMessage(const uint32_t user_id, const std::string &buf, std::vector<SateParam> &sates, const int &level);
    bool SendGroupMessage(const uint32_t group_id, const std::string&buf, std::vector<int> &beams, int level);
private:
    SendHelper(){}
    static SendHelper* instance_;
private:
    std::map<int32_t, Address> schedule_address_;
};


#endif /*define send_helper_h_*/