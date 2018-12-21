#include "LogicInterfaceHandler.h"
#include "proto/im.pb.h"
#include "logic/protocol.h"
#include <string>

using namespace im;

LogicInterfaceHandler::LogicInterfaceHandler(Controller* controller):
    controller_(controller) {
}

void AccessToLogic(const std::string& request) {
    if (request.emtpy()) {
        return;
    }
    ControlHead* control_head = (ControlHead*)request.c_str();

    switch (control_head->type) {
        case Login:
            controller_->mobile_manager_->ProcessLogin(control_head);
            break;
        case Logout:
            controller_->mobile_manager_->ProcessLogout(control_head);
            break; 
        case MOBILE_MANAGER_REQUEST:
            controller_->mobile_manager_->ProcessMobileRequest(control_head);
            break;
        case SIMPLE_MESSAGE:
            controller_->message_manager_->ProcessSimpleMessage(control_head);
            break;
        case FORWARD_NOACK_MESSAGE:
            controller_->message_manager_->ProcessForwardNoAckMessage(control_head);
            break;
        case BACKWARD_NOACK_MESSAGE:
            controller->message_manager_->ProcessBackwardNoAckMessage(control_head);
            break;
        case COMPLETE_MESSAGE:
            controller->message_manager_->ProcessCompleteMessage(control_head);
            break;
        case INQUIRE_MESSAGE_REQUEST:
            break;
        case RECEIPTE:
            break;
        default:
            break;
    }
}
