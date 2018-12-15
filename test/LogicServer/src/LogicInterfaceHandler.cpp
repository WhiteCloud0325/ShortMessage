#include "LogicInterfaceHandler.h"
#include "proto/im.pb.h"
#include <string>

using namespace im;

void LogicInterfaceHandler::AckMessage(const Response&message) {
    if (!message.__isset.content) {
        return;
    }
    MessageItem mi;
    if (!mi.ParseFromString(message.content)) {
        return;
    }
    std::string content = mi.DebugString();
    std::cout << content << std::endl;
}

