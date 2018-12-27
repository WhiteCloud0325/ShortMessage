#include "LogicInterfaceHandler.h"
#include "logic/protocol.h"
#include <string>
#include <iostream>
#include <arpa/inet.h>
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
    uint32_t to_id = ntohl(control_head->to_id);
    uint32_t from_id = ntohl(control_head->from_id);
    uint16_t frame_id = ntohs(control_head->frame_id);
    uint8_t type = control_head->type;
    std::cout << "msg: " << response.content << std::endl;
    std::cout << "to_id: " << to_id << std::endl;
    std::cout << "from_id: "  << from_id << std::endl;
    std::cout << "frame_id: " << frame_id << std::endl;
    std::cout << "type: "   << (int)type << std::endl;
    Response* rept = NULL;
    switch (control_head->type) {
        case Login:
            break;
        case Logout:
            break; 
        case MOBILE_MANAGER_REQUEST:
            break;
        case SIMPLE_MESSAGE:
            std::cout << "content: " <<control_head->content << std::endl;
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
            rept = (Response*) (response.content.c_str());
            std::cout << "receipt_type: "  << (int) rept->receipt_type << std::endl;
            std::cout << "receipt_indicate: " << (int)rept->receipt_indicate << std::endl;
            break;
        default:
            break;
    }
}

