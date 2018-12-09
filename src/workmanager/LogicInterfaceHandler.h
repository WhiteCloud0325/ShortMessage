#ifndef LogicInterfaceHandler_H_
#define LogicInterfaceHandler_H_

#include "thrift/LogicInterface.h"
#include "workmanager/controller.h"

using namespace im;
class LogicInterfaceHandler: virtual public LogicInterfaceIf {
public:
    LogicInterfaceHandler(Controller *controller);
    virtual void SendMessage(const Request& message);
    virtual void AckMessage(const Response& message){};
private:
    Controller *controller_;
};


#endif /* #difine LogicInterfaceHandler_H_*/
