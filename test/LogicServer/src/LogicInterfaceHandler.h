#ifndef LogicInterfaceHandler_H_
#define LogicInterfaceHandler_H_

#include "thrift/LogicInterface.h"

using namespace im;
class LogicInterfaceHandler: virtual public LogicInterfaceIf {
public:
   // LogicInterfaceHandler();
    virtual void AccessToLogic(const std::string& request){};
    virtual void LogicToAccess(const AccessMessage& response);
private:
};


#endif /* #difine LogicInterfaceHandler_H_*/
