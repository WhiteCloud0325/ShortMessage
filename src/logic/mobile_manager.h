#ifndef mobile_manager_h_
#define mobile_manager_h_

#include "tools/database.h"
#include "tools/qos_send_daemon.h"
#include "logic/protocol.h"
class MobileManager {
public:
    MobileManager(): database_(NULL){}
    ~MobileManager(){}
    bool Init(Database *database);
    void ProcessLogin(const ControlHead * control_head);
    void ProcessLogout(const ControlHead * control_head);
    void ProcessMobileRequest(const ControlHead * control_head);
private:
    Database * database_;
};


#endif  /* define mobile_manager_h_*/
