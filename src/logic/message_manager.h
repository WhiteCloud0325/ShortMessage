#ifndef message_manager_h_
#define message_manager_h_
#include "tools/database.h"
#include "logic/protocol.h"

class MessageManager {
public:
    MessageManager():database_(NULL){}
    ~MessageManager() {}
    bool Init(Database *database);
    void ProcessSimpleMessage(ControlHead *control_head, Connection_T conn);
    void ProcessForwardNoAckMessage(ControlHead *control_head, Connection_T conn);
    void ProcessBackwardNoAckMessage(ControlHead *control_head, Connection_T conn);
    void ProcessCompleteMessage(ControlHead *control_head, Connection_T conn);
    void ProcessReceipt(ControlHead *control_head, Connection_T conn);
private:
    Database *database_;
};

#endif /* define message_manager_h_ */

