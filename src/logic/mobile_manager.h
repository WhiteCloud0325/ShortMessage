#ifndef mobile_manager_h_
#define mobile_manager_h_

#include "tools/database.h"
#include "tools/qos_send_daemon.h"
#include "tools/protocol.h"
class MobileManager {
public:
    MobileManager(): database_(NULL){}
    ~MobileManager(){}
    bool Init(Database *database);
private:
    Database * database_;
};


#endif  /* define mobile_manager_h_*/
