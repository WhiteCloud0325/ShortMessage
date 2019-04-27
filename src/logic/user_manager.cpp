#include "logic/user_manager.h"
#include "tools/logger.h"
#include "workmanager/send_helper.h"
#include "logic/coder.h"
#include <arpa/inet.h>
#include <time.h>
#include <boost/timer/timer.hpp>
#include <boost/chrono.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

void UserManager::Register(ControlHead * control_head, Connection_T conn) {
    
}

void UserManager::FriendAdd(ControlHead * control_head, Connection_T conn) {
    
}

void UserManager::FriendDelete(ControlHead * control_head, Connection_T conn) {

}

void UserManager::FriendList(ControlHead * control_head, Connection_T conn) {
    
}

void UserManager::GroupCreate(ControlHead * control_head, Connection_T conn) {
    uint32_t user_id = ntohl(control_head->from_id);
    uint16_t frame_id = ntohs(control_head->frame_id);
    char *pos = control_head->content;

    std::string group_name = std::string(pos, 40);
    pos += 40;
    uint32_t num = *(uint32_t*)pos;
    pos +=4;
    std::vector<uint32_t> members;
    for (uint32_t i = 0; i < num; ++i) {
        members.push_back(*(uint32_t*)pos);
        pos += 4;
    }
    int group_id = database_->GroupCreate(conn, user_id, group_name);

    MessageItem message;
    message.to_id = user_id;
    message.from_id = 0;
    message.frame_id = control_head->frame_id;
    message.type = 0x31;
    message.retain = control_head->retain;
    message.content = std::string((char*)&group_id, 4);
    if (group_id == -1) {
        std::vector<SateParam> sates;
        database_->GetSateCover(conn, user_id, sates);
        if (!sates.empty()) {
            std::string response = MessageEncode(message);
            SendHelper::GetInstance()->SendMessage(user_id, response, sates, 10);
        }
    }
    else {
        message.to_id = group_id;
        members.push_back(user_id);
        if(!database_->GroupAddMembers(conn, group_id, members)) {
            return;
        }    
        std::vector<int> beams;
        database_->GroupGetSateCover(conn, group_id, beams);
        if (beams.empty()) {
            return;
        } 
        std::string response = MessageEncode(message);
        SendHelper::GetInstance()->SendGroupMessage(group_id, buf, beams, 10);
    }
}

void UserManager::GroupAddUser(ControlHead * control_head, Connection_T conn) {
    uint32_t user_id = ntohl(control_head->from_id);
    uint16_t frame_id = ntohs(control_head->frame_id);
    char* pos = control_head->content;
    uint32_t group_id =  ntohl(*(uint32_t*)pos);
    pos += 4;
    int num = *(int*)pos;
    pos += 4;
    std::vector<uint32_t> members;
    for (int i = 0; i < num; ++i) {
        members.push_back(*(uint32_t*)pos);
        pos += 4;
    }
    bool res = database_->GroupAddMembers(conn, group_id, members);
    MessageItem message;
    message.to_id = user_id;
    message.from_id = 0;
    message.frame_id = frame_id;
    message.type = 0x33;
    message.retain = control_head->retain;
    message.content = std::string(control_head->content,  4 * (num + 2));
    if (!res) {
        std::vector<SateParam> sates;
        database_->GetSateCover(conn, user_id, sates);
        if (!sates.empty()) {
            int temp = -1;
            message.content = std::string((char*)&temp, 4);
            std::string response = MessageEncode(message);
            SendHelper::GetInstance()->SendMessage(user_id, response, sates, 10);
        }
    }
    else {
        std::vector<int> beams;
        database_->GroupGetSateCover(conn, group_id, beams);
        if (beams.empty()) {
            return;
        }
        message.to_id = group_id;
        std::string response = MessageEncode(message);
        SendHelper::GetInstance()->SendGroupMessage(group_id, response, beams, 10);
    }
}

/*void UserManager::GroupQuitUser(ControlHead * control_head, Connection_T conn) {
    uint32_t user_id = control_head->from_id;
    uint16_t frame_id = control_head->frame_id;
    char *pos = control_head->content;
    uint32_t group_id = *(uint32_t*) pos;
    bool res = database_->GroupDeleteMember(conn, group_id, user_id);
    MessageItem message;
    message.to_id = user_id;
    message.from_id = 0;
    message.frame_id = frame_id;
    message.type = GROUP_QUIT_RESPONSE;
    message.retain = control_head->retain;
    message.content = std::string(control_head->content, 4);

    if (res) {
        std::vector<uint32_t> members = database_->GroupListUserId(conn, group_id);
        members.push_back(user_id);
        for (int i = 0; i < members.size(); ++i) {
            std::vector<SateParam> sates;
            database_->GetSateCover(conn, members[i], sates);
            if (sates.empty()) {
                continue;
            }
            message.to_id = members[i];
            std::string response = MessageEncode(message);
            SendHelper::GetInstance()->SendMessage(members[i], response, sates, 10);
        }
    }
}*/

void UserManager::GroupDeleteMember(ControlHead * control_head, Connection_T conn) {
    uint32_t frame_id = ntohs(control_head->frame_id);
    char *pos = control_head->content;
    uint32_t group_id = ntohl(*(uint32_t*)pos);
    pos += 4;
    uint32_t num = *(uint32_t*)pos;
    if (num == 0) {
        return;
    }
    pos += 4;
    std::vector<uint32_t> members;
    for (uint32_t i = 0; i < num; ++i) {
        members.push_back(*(pos + 4));
        pos += 4;
    }
    
    bool res = database_->GroupDeleteMembers(conn, group_id, members);
    if (!res) {
        return;
    }
    std::vector<int> beams;
    database_->GroupGetSateCover(conn, group_id, beams);
    if (beams.empty()) {
        return;
    }
    MessageItem message;
    message.to_id = control_head->group_id; // I really don not know what should be 
    message.from_id = 0;
    message.frame_id = frame_id; //so is this 
    message.type = GROUP_DELETE_MEMBER_RESPONSE;
    message.retain = control_head->retain;
    message.content = std::string(control_head->content, 4 * (2 + num));
    std::string response = MessageEncode(message);
    SendHelper::GetInstance()->SendGroupMessage(group_id, response, beams, 10);
}








 
