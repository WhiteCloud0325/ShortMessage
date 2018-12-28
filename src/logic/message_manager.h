#ifndef message_manager_h_
#define message_manager_h_
#include "tools/database.h"
#include "logic/protocol.h"

class MessageManager {
public:
    MessageManager(): database_(NULL) {}
    ~MessageManager() {}
    bool Init(Database *database);
    void ProcessSimpleMessage(ControlHead *control_head);
    void ProcessForwardNoAckMessage(ControlHead *control_head);
    void ProcessBackwardNoAckMessage(ControlHead *control_head);
    void ProcessCompleteMessage(ControlHead *control_head);
    void ProcessReceipt(ControlHead *control_head);
private:
    Database *database_;
};

#endif /* define message_manager_h_ */

