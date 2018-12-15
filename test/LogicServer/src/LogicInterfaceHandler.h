#ifndef LogicInterfaceHandler_H_
#define LogicInterfaceHandler_H_

#include "thrift/LogicInterface.h"

using namespace im;
class LogicInterfaceHandler: virtual public LogicInterfaceIf {
public:
    LogicInterfaceHandler(){}
    virtual void SendMessage(const Request& message){};
    virtual void AckMessage(const Response& message);
};


#endif /* #difine LogicInterfaceHandler_H_*/
