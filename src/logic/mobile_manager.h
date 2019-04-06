#ifndef mobile_manager_h_
#define mobile_manager_h_

#include "tools/database.h"
#include "tools/qos_send_daemon.h"
#include "logic/protocol.h"
#include "logic/manager.h"
class MobileManager: public {
public:
    MobileManager():{}
    ~MobileManager(){}
    void ProcessLogin(const ControlHead * control_head, Connection_T conn);
    void ProcessLogout(const ControlHead * control_head, Connection_T conn);
    void ProcessMobileRequest(const ControlHead * control_head, Connection_T conn);
};


#endif  /* define mobile_manager_h_*/
