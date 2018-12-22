#include "logic/coder.h"
#include <string.h>
#include <string>
#include "logic/protocol.h"

std::string ResponseEncode(const Response &response) {
    char buf[1024] = {0};
    char* pos = buf;
    *(uint32_t *)pos = response.to_id;
    pos +=4;
    *(uint32_t *)pos = response.from_id;
    pos +=4;
    *(uint16_t *)pos = response.frame_id;
    pos +=2;
    *(uint8_t*)pos++ = response.type;
    *(uint8_t*)pos++ = response.retain;
    *(uint8_t*)pos++ = response.receipt_type;
    *(uint8_t*)pos++ = response.receipt_indicate;
    std::string res(buf, (pos - buf) + 1);
    return res;
}

std::string MessageEncode(const ControlHead* control_head) {
    char buf[1024] = {0};
    char *pos = buf;
    *(uint32_t*)pos = control_head->to_id;
    pos +=4;
    *(uint32_t*)pos = control_head->from_id;
    pos +=4;
    *(uint16_t*)pos = control_head->frame_id;
    pos +=2;
    *(uint8_t*)pos++ = control_head->type;
    *(uint8_t*)pos++ = control_head->retain;
    strcpy(pos, control_head->content);
    pos += strlen(control_head->content) + 1;
    std::string res(buf, pos - buf);
    return res;
}
