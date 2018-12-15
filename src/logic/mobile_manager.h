#ifndef mobile_manager_h_
#define mobile_manager_h_

#include "tools/database.h"
#include "tools/qos_send_daemon.h"
#include "tools/protocol.h"
class MobileManager {
public:
    MobileManager(): database_(NULL){}
    ~MobileManager(){}
    bool Init(Database *database, QosSendDaemon *qos_send_daemon);
    void ProcessC2SMessage(const std::string &request);
    void ProcessACK(const std::string &ack);

private:
    Database * database_;
    QosSendDaemon *qos_send_daemon_;
};


#endif  /* define mobile_manager_h_*/
