#include "LogicInterfaceHandler.h"
#include <string>
#include <iostream>
#include <string.h>
#include "tools/priority_queue.h"
#include <arpa/inet.h>
using namespace im;

/*LogicInterfaceHandler::LogicInterfaceHandler(Controller* controller):
    controller_(controller) {
}*/

void LogicInterfaceHandler::LogicToAccess(const AccessMessage& response) {
    uint8_t beam_id = response.beam_id;
    uint8_t sate_id = response.sate_id;
    int level = response.level;
    int len = response.content.size();
    len -= 4;
    char buf[1024] = {0};
    char *write_pos = buf;
    const char *read_pos = response.content.c_str();

   /* uint32_t uid = *(uint32_t*)read_pos;
    uid = ntohl(uid);
    uint32_t sid = *(uint32_t*)(read_pos + 4);
    sid = ntohl(sid);
    uint16_t fid = *(uint16_t*)(read_pos + 8);
    fid = ntohs(fid);
    uint8_t type = *(uint8_t*)(read_pos + 10);
    std::cout << uid << std::endl;
    std::cout << sid << std::endl;
    std::cout << fid << std::endl;
    std::cout << (int)type << std::endl;*/

    memcpy(write_pos, read_pos, 4);
    write_pos +=4;
    read_pos +=4;
    *(uint8_t*)write_pos++ = sate_id;
    *(uint8_t*)write_pos++ = beam_id;
    write_pos +=26;
    memcpy(write_pos, read_pos, len);
    
    
    /*uint32_t sid = *(uint32_t*)write_pos;
    uint16_t fid = *(uint16_t*)(write_pos + 4);
    uint8_t  type = *(uint8_t*)(write_pos + 2);
    std::cout << sid << std::endl;
    std::cout << fid << std::endl;
    std::cout << type << std::endl;*/
    
    write_pos += len;
    std::string str(buf, write_pos - buf);
    //controller_->priority_queue_.Push(level, str);
    printf("receive message\n");
    controller_->priority_queue_.Push(level, response.content);

}

