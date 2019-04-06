#include "LogicInterfaceHandler.h"
#include "logic/protocol.h"
#include <string>
#include <iostream>
#include <sys/time.h>
#include <time.h>
using namespace im;

LogicInterfaceHandler::LogicInterfaceHandler(Controller* controller):
    controller_(controller) {
}

void LogicInterfaceHandler::AccessToLogic(const std::string& request) {
    if (request.empty()) {
        return;
    }
  /*  for (int i = 0; i < request.size(); ++i) {
        printf("%02x ", (uint32_t)request[i]);
    }
    printf("\n");*/
    controller_->Push(request);
}
