#include "logic/mobile_manager.h"
#include "workmanager/send_helper.h"
#include "tools/logger.h"
#include "logic/coder.h"
#include <arpa/inet.h>

using namespace im;

bool MobileManager::Init(Database *database) {
    database_ = database;
    return true;
}

void MobileManager::ProcessLogin(const ControlHead* control_head) {
    uint32_t user_id = ntohl(control_head->from_id);
    MobileRequest *mobile_request = (MobileRequest*) (control_head->content);
    uint8_t *pos = (uint8_t *) (mobile_request->content);
    Satellite sate;
    sate.grand_cover = *pos++;
    sate.sat_cover_num = *pos++;
    if (sate.sat_cover_num == 0) {
        LOG_INFO("ProcessLogin sat_cover_num = 0: user_id = %u", user_id);
        return;
    }
    for(int i = 0; i < sate.sat_cover_num; ++i) {
        SateParam sate_param;
        sate_param.sate_id = (int32_t) *pos++;
        sate_param.beam_id = (int32_t) *pos++;
        sate_param.snr = *(float*)pos;
        pos+=4;
        sate.sates_param.push_back(sate_param);
    }
    
    Response response;
    response.to_id = control_head->from_id;
    response.from_id = control_head->to_id;
    response.frame_id = control_head->frame_id;
    response.type = 0x80;
    response.retain = control_head->retain;
    response.receipt_type = 0x01;
    response.receipt_indicate = 0x00;
    LOG_DEBUG("ProcessLogin: user_id=%ld||beam_num", user_id, sate.sat_cover_num);
    Connection_T conn = database_->GetConnection();
    if (conn == NULL) {
        return;
    }
    bool res = database_->IsExistUser(conn, user_id);
    if (res) {
        res = database_->UpdateSateCover(conn, user_id, sate);
        if (res) {
            response.receipt_type = 0x00;
            int num = database_->GetOfflineMessageNum(conn, user_id);
            if (num >= 0 && num < 255) {
                response.receipt_indicate = (uint8_t)num;
            }
            else {
                response.receipt_indicate = 0xFF;
            }
        }
        else {
            response.receipt_type = 0x01;
            response.receipt_indicate = 0x00;
        }
    } 
    else {
        response.receipt_type = 0x01;
        response.receipt_indicate = 0x00;
    }
    Connection_close(conn);
    std::string str =  ResponseEncode(response);
    SendHelper::GetInstance()->SendMessage(user_id, str, sate.sates_param);

}

void MobileManager::ProcessLogout(const ControlHead *control_head) {
    MobileRequest *mobile_request = (MobileRequest*) (control_head->content);
    uint32_t user_id = ntohl(control_head->from_id);
    Response response;
    response.to_id = control_head->from_id;
    response.from_id = control_head->to_id;
    response.frame_id = control_head->frame_id;
    response.type = 0x80;
    response.retain = control_head->retain;
    response.receipt_type = 0x02;
    response.receipt_indicate = 0x00;
    LOG_DEBUG("ProcessLogout: user_id=%u", user_id);

    Satellite sate;
    sate.sat_cover_num = 0;
    Connection_T conn = database_->GetConnection();
    if (conn == NULL) {
        return;
    }
    std::vector<SateParam> sates = database_->GetSateCover(conn, user_id);
    database_->UpdateSateCover(conn, user_id, sate);
    Connection_close(conn);
    std::string str = ResponseEncode(response);
    SendHelper::GetInstance()->SendMessage(user_id, str, sates);

}

void MobileManager::ProcessMobileRequest(const ControlHead *control_head) {
    uint32_t user_id = ntohl(control_head->from_id);
    MobileRequest *mobile_request = (MobileRequest*) (control_head->content);
    uint8_t *pos = (uint8_t*) (mobile_request->content);
    Satellite sate;
    sate.grand_cover = *pos++;
    sate.sat_cover_num = *pos++;
    if (sate.sat_cover_num == 0) {
        LOG_INFO("ProcessMobileRequest sat_cover_num = 0: user_id = %u", user_id);
        return;
    }
    for(int i = 0; i < sate.sat_cover_num; ++i) {
        SateParam sate_param;
        sate_param.sate_id = (int32_t) *pos++;
        sate_param.beam_id = (int32_t) *pos++;
        sate_param.snr = *(float*)pos;
        pos+=4;
        sate.sates_param.push_back(sate_param);
    }
    
    Response response;
    response.to_id = control_head->from_id;
    response.from_id = control_head->to_id;
    response.frame_id = control_head->frame_id;
    response.type = 0x80;
    response.retain = control_head->retain;
    response.receipt_type = 0x03;
    response.receipt_indicate = 0x00;
    LOG_DEBUG("ProcessLogin: user_id=%ld||beam_num", user_id, sate.sat_cover_num);
    Connection_T conn = database_->GetConnection();
    if (conn == NULL) {
        return;
    }
    database_->UpdateSateCover(conn, user_id, sate);
    Connection_close(conn);
    std::string str =  ResponseEncode(response);
    SendHelper::GetInstance()->SendMessage(user_id, str, sate.sates_param);
}


