#include "LogicInterfaceHandler.h"
#include "logic/protocol.h"
#include <string>
#include <iostream>
#include <string.h>
using namespace im;

/*LogicInterfaceHandler::LogicInterfaceHandler(Controller* controller):
    controller_(controller) {
}*/

void LogicInterfaceHandler::LogicToAccess(const AccessMessage& response) {
    uint8_t beam_id = response.beam_id[0];
    uint8_t sate_id = 1;
    int len = response.size();
    len -= 4;
    char buf[1024] = {0};
    char *write_pos = buf;
    char *read_pos = response.content.c_str();
    memcpy(write_pos, read_pos, 4);
    write_pos +=4;
    read_pos +=4;
    *(uint8_t*)write_pos++ = sate_id;
    *(uint8_t*)write_pos++ = beam_id;
    write_pos +=26;
    memcpy(write_pos, read_pos, len);
    QueueItem item(6,buf);
    contronller_->priority_queue_.Push(item)
}
