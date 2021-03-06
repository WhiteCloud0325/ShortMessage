#include "LogicInterfaceHandler.h"
#include "logic/protocol.h"
#include <string>
#include <iostream>
#include <sys/time.h>
#include <time.h>
using namespace im;

LogicInterfaceHandler::LogicInterfaceHandler(Controller* controller):
    controller_(controller) {
}

void LogicInterfaceHandler::AccessToLogic(const std::string& request) {
    if (request.empty()) {
        return;
    }
    /*for (int i = 0; i < request.size(); ++i) {
        printf("%02x ", (uint8_t)request[i]);
    }
    printf("\n");*/
    timeval start;
    gettimeofday(&start, NULL);
    ControlHead* control_head = (ControlHead*)request.c_str();
    switch (control_head->type) {
        case Login:
            controller_->mobile_manager_.ProcessLogin(control_head);
            break;
        case Logout:
            controller_->mobile_manager_.ProcessLogout(control_head);
            break; 
        case MOBILE_MANAGER_REQUEST:
            controller_->mobile_manager_.ProcessMobileRequest(control_head);
            break;
        case SIMPLE_MESSAGE:
            controller_->message_manager_.ProcessSimpleMessage(control_head);
            break;
        case FORWARD_NOACK_MESSAGE:
            controller_->message_manager_.ProcessForwardNoAckMessage(control_head);
            break;
        case BACKWARD_NOACK_MESSAGE:
            controller_->message_manager_.ProcessBackwardNoAckMessage(control_head);
            break;
        case COMPLETE_MESSAGE:
            controller_->message_manager_.ProcessCompleteMessage(control_head);
            break;
        case INQUIRE_MESSAGE_REQUEST:
            break;
        case RECEIPTE:
            controller_->message_manager_.ProcessReceipt(control_head);
            break;
        default:
            break;
    }
    timeval end;
    gettimeofday(&end, NULL);
    uint32_t cost_time = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
    LOG_DEBUG("AccessToLogic: time=%ld", cost_time);
}
