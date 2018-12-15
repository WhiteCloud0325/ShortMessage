#ifndef message_manager_h_
#define message_manager_h_
#include "tools/database.h"

class MessageManager {
public:
    MessageManager(): database_(NULL),qos_send_daemon_(NULL) {}
    ~MessageManager() {}
    bool Init(Database *database, QosSendDaemon *qos_send_daemon);
    void ProcessC2SMessage(const std::string &request);
    void ProcessACK(const std::string &ack);
private:
    Database *database_;
    QosSendDaemon *qos_send_daemon_;
};

#endif /* define message_manager_h_ */

