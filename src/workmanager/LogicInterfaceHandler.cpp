#include "LogicInterfaceHandler.h"
#include "proto/im.pb.h"
#include <string>

using namespace im;

LogicInterfaceHandler::LogicInterfaceHandler(Controller* controller):
    controller_(controller) {
}

void LogicInterfaceHandler::SendMessage(const Request& message) {
    if ( !message.__isset.type || !message.__isset.content) 
        return;
    int32_t type = message.type;
    std::string request = message.content;
    std::string response, to_ip;
    int32_t to_type;
    int64_t to_uid;

    switch (type) {
        case REGISTER_REQUEST:
            controller_->user_manager_.UserRegister(request, to_type, to_uid, response, to_ip);
            break;
        case REGISTER_RESPONSE:
            break; 
        case LOGIN_REQUEST:
            controller_->user_manager_.UserAuth(request, to_type, to_uid, response, to_ip);
            break; 
        case LOGIN_RESPONSE:
            break;
        case LOGOUT_REQUEST: 
            break;
        case FRIEND_LIST_REQUEST:
            break;
        case FRIEND_LIST_RESPONSE: 
            break;
        case FRIEND_FIND_REQUEST:
            break;
        case FRIEND_FIND_RESPONSE: 
            break;
        case FRIEND_ADD_REQUEST:
            break;
        case FRIEND_ADD_RESPONSE: 
            break;
        case FRIEND_DELETE_REQUEST: 
            break;
        case FRIEND_DELETE_RESPONSE:  
            break;
        case GROUP_CREATE_REQUEST:
            break;
        case GROUP_CRAETE_RESPONSE: 
            break;
        case GROUP_ADDMEMBER_REQUEST: 
            break;
        case GROUP_ADDMEMBER_RESPONSE: 
            break;
        case GROUP_DELETEMEMBER_REQUEST: 
            break;
        case GROUP_DELETEMEMBER_RESPONSE: 
            break;
        case GROUP_DELETE_REQUEST: 
            break;
        case GROUP_DELETE_RESPONSE: 
            break;
        case GROUP_EXIT_REQUESET: 
            break;
        case GROUP_EXIT_RESPONSE: 
            break;
        case MESSAGE_FRIEND_SEND_REQUEST: 
            break;
        case MESSAGE_FRIEND_RESPONSE: 
            break;
        case MESSAGE_GROUP_SEND_REQUEST: 
            break;
        case MESSAGE_GROUP_SEND_RESPONSE: 
            break;
        case MESSAGE_FRIEND_NOTIFY_REQUEST: 
            controller_->message_manager_->ProcessC2SMessage(request);
            break;
        case MESSAGE_FRIEND_NOTIFY_RESPONSE: 
            controller_->message_manager_->ProcessACK(request);
            break;
        case MESSAGE_GROUP_NOTIFY_REQUEST:
            break;
        case MESSAGE_GROUP_NOTIFY_RESPONSE: 
            break;
        default:
            break;
    }
}
