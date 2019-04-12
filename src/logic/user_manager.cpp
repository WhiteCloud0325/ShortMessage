#include "logic/user_manager.h"
#include "tools/logger.h"
#include "workmanager/send_helper.h"
#include "logic/coder.h"

void UserManager::Register(const ControlHead * control_head, Connection_T conn) {
    
}

void UserManager::FriendAdd(const ControlHead * control_head, Connection_T conn) {
    
}

void UserManager::FriendDelete(const ControlHead * control_head, Connection_T conn) {

}

void UserManager::FriendList(const ControlHead * control_head, Connection_T conn) {
    
}

void UserManager::GroupCreate(const ControlHead * control_head, Connection_T conn) {
    uint32_t user_id = control_head->from_id;
    uint16_t frame_id = control_head->frame_id;
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
    int group_id = database.GroupCreate(conn, user_id, group_name, members);
    MessageItem message;
    message.to_id = user_id;
    message.from_id = 0;
    message.frame_id = control_head->frame_id;
    message.type = 0x31;
    message.retain = control_head->retain;
    message.content = std::string((char*)&group_id, 4);
    if (group_id == -1) {
        std::vector<SateParam> sates = database_->GetSateCover(conn, user_id);
        if (!sates.empty()) {
            std::string response = MessageEncode(message);
            SendHelper::GetInstance()->SendMessage(user_id, response, sates, 10);
        }
    }
    else {
        members.push_back(user_id);
        for (int i = 0; i < members.size(); ++i) {
            std::vector<SateParam> sates = database_->GetSateCover(conn, members[i]);
            if (sates.empty()) {
                continue;
            }
            message.to_id = members[i];
            std::string response = MessageEncode(message);
            SendHelper::GetInstance()->SendMessage(members[i], response, sates, 10);
        }
    }
}

void UserManager::GroupAddUser(const ControlHead * control_head, Connecton_T conn) {
    uint32_t user_id = control_head->from_id;
    uint16_t frame_id = control_head->frame_id;
    char* pos = control_head->content;
    uint32_t group_id =  *(uint32_t*)pos;
    pos += 4;
    int num = *(int*)pos;
    pos += 4;
    std::vector<uint32_t> members;
    for (i = 0; i < num; ++i) {
        members.push_back(*(uint32_t*)pos);
        pos += 4;
    }
    bool res = database_->GroupAddMember(conn, group_id, members);
    MessageItem message;
    message.to_id = user_id;
    message.from_id = 0;
    message.frame_id = frame_id;
    message.type = 0x33;
    message.retain = control_head->retain;
    message.content = std::string(control_head->content,  4 * (num + 2));
    if (!res) {
        std::vector<SateParam> sates = database_->GetSateCover(conn, user_id);
        if (!sates.empty()) {
            int temp = -1;
            message.content = std::string((char*)&temp, 4);
            std::string response = MessageEncode(message);
            SendHelper::GetInstance()->SendMessage(user_id, response, sates, 10);
        }
    }
    else {
        std::vector<uint32_t> all_members = database_->GroupListUserId(conn, group_id);
        for (int i = 0; i < all_members.size(); ++i) {
            std::vector<SateParam> sates = database_->GetSateCover(conn, all_members[i]);
            if (sates.empty()) {
                continue;
            }
            message.to_id = all_members[i];
            std::string response = MessageEncode(message);
            SendHelper::GetInstance()->SendMessage(all_member[i], response, sates, 10);
        }
    }
}

void Database::GroupQuitUser(const ControlHead * control_head, Connetion_T conn) {
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
            std::vector<SateParam> sates = database_->GetSateCover(conn, members[i]);
            if (sates.empty()) {
                continue;
            }
            message.to_id = members[i];
            std::string response = MessageEncode(message);
            SendHelper::GetInstance()->SendMessage(members[i], response, sates, 10);
        }
    }
}

void Database::GroupDeleteMember(const ControlHead * control_head, Connection_T conn) {
    uint32_t frame_id = control_head->frame_id;
    char *pos = control_head->content;
    uint32_t group_id = *(uint32_t*)pos;
    pos += 4;
    uint32_t num = *(uint32_t*)pos;
    if (num == 0) {
        return;
    }
    pos += 4;
    std::vector<uint32_t> memebers;
    for (uint32_t i = 0; i < num; ++i) {
        members.push_back(*(pos + 4));
        pos += 4;
    }
    std::vector<uint32_t> all_members = database_->GroupListUserId(conn, group_id);
    bool res = database_->GroupDeleteMembers(conn, group_id, members);
    if (!res) {
        return;
    }
    MessageItem message;
    message.to_id = control_head->from_id;
    message.from_id = control_head->to_id;
    message.frame_id = frame_id;
    message.type = GROUP_DELETE_MEMBER_RESPONSE;
    message.retain = control_head->retain;
    message.content = std::string(control_head->content, 4 * (2 + num));
    for (auto user_id: all_members) {
        std::vector<SateParam> sates = GetSateCover(conn, user_id);
        if (sates.empty()) {
            continue;
        }
        message.to_id = user_id;
        std::string response = MessageEncode(message);
        SendHelper::GetInstance()->SendMessage(response);
    }
}








 
