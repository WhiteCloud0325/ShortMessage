#ifndef LogicInterfaceHandler_H_
#define LogicInterfaceHandler_H_

#include "thrift/LogicInterface.h"
#include "controller.h"
using namespace im;
class LogicInterfaceHandler: virtual public LogicInterfaceIf {
public:
    LogicInterfaceHandler(): controller_(NULL){};
    LogicInterfaceHandler(Controller* controller): controller_(controller){}
    virtual void AccessToLogic(const std::string& request){};
    virtual void LogicToAccess(const AccessMessage& response);
private:
    Controller *controller_;
};


#endif /* #difine LogicInterfaceHandler_H_*/
