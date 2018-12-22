#ifndef coder_h_
#define coder_h_

#include <string.h>
#include <string>
#include "logic/protocol.h"
std::string ResponseEncode(const Response &response); 

std::string MessageEncode(const ControlHead* control_head);

std::string MessageEncode(const MessageItem &message); 

#endif /* define coder_h_*/
