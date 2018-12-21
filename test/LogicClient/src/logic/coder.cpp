#include "logic/coder.h"
#include <string.h>
#include <string>
#include "logic/protocol.h"

std::string ResponseEncode(const Response &response) {
    uint8_t buf[1024] = {0};
    uint8_t* pos = buf;
    *(uint32_t *)pos = response.to_id;
    pos +=4;
    *(uint32_t *)pos = response.from_id;
    pos +=4;
    *(uint16_t *)pos = response.frame_id;
    pos +=2;
    *(pos++) = response.type;
    *(pos++) = response.retain;
    *(pos++) = response.receipt_type;
    *(pos++) = response.receipt_indicate;
    std::string res((char*)buf);
    return res;
}

std::string MessageEncode(const ControlHead* control_head) {
    uint8_t buf[1024] = {0};
    uint8_t *pos = buf;
    *(uint32_t*)pos = control_head->to_id;
    pos +=4;
    *(uint32_t*)pos = control_head->from_id;
    pos +=4;
    *(uint16_t*)pos = control_head->frame_id;
    pos +=2;
    *pos++ = control_head->type;
    *pos++ = control_head->retain;
    strcpy((char*) pos, control_head->content);
    std::string res((char*)buf);
    return res;
}