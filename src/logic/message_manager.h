#ifndef message_manager_h_
#define message_manager_h_
#include "logic/protocol.h"
#include "logic/manager.h"

class MessageManager: public Manager {
public:
    MessageManager(){}
    ~MessageManager() {}
    void ProcessSimpleMessage(ControlHead *control_head, Connection_T conn);
    void ProcessForwardNoAckMessage(ControlHead *control_head, Connection_T conn);
    void ProcessBackwardNoAckMessage(ControlHead *control_head, Connection_T conn);
    void ProcessCompleteMessage(ControlHead *control_head, Connection_T conn);
    void ProcessReceipt(ControlHead *control_head, Connection_T conn);
    void ProcessGroupMessage(ControlHead * control_head, Connection_T conn);
    void ProcessGroupMessageReceipt(ControlHead * control_head, Connection_T conn);
    void ProcessGroupMessagePull(ControlHead* control_head, Connection_T conn);
    void ProcessInquireMessage(ControlHead* control_head, Connection_T conn);
};

#endif /* define message_manager_h_ */

