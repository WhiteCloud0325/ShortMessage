#include "LogicInterfaceHandler.h"
#include "logic/protocol.h"
#include <string>
#include <iostream>

using namespace im;

/*LogicInterfaceHandler::LogicInterfaceHandler(Controller* controller):
    controller_(controller) {
}*/

void LogicInterfaceHandler::LogicToAccess(const AccessMessage& response) {
    std::cout << "uid:" << response.uid << std::endl;;
    std::cout << "beam:";
    for (auto beam_id: response.beam_id) {
        std::cout << beam_id << ", ";
    }
    std::cout <<std::endl;
    ControlHead *control_head = (ControlHead*) response.content.c_str();
    switch (control_head->type) {
        case Login:
            break;
        case Logout:
            break; 
        case MOBILE_MANAGER_REQUEST:
            break;
        case SIMPLE_MESSAGE:
            std::cout << control_head->content << std::endl;
            break;
        case FORWARD_NOACK_MESSAGE:
            break;
        case BACKWARD_NOACK_MESSAGE:
            break;
        case COMPLETE_MESSAGE:
            break;
        case INQUIRE_MESSAGE_REQUEST:
            break;
        case RECEIPTE:
            break;
        default:
            break;
    }
}

