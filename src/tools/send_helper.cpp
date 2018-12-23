#include "tools/send_helper.h"
#include "tools/logger.h"
#include <boost/shared_ptr.hpp>
#include <thrift/Thrift.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

using namespace apache;
using namespace apache::thrift;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;

SendHelper* SendHelper::instance_ = NULL;

bool SendHelper::Init(libconfig::Setting &setting) {
    try {
        if (!setting.lookupValue("port", logic_port_)) {
            return false;
        }
        if (!setting.lookupValue("ip", logic_ip_)) {
            return false;
        }
    }
    catch (...) {
        LOG_INFO("SendHelper Init Exception");
        return false;
    }
    return true;
}

void SendHelper::SendMessage(const std::string &buf) {
    try {
        boost::shared_ptr<TSocket> client_socket(new TSocket(logic_ip_, logic_port_));
        boost::shared_ptr<TTransport> client_transport(new TBufferedTransport(client_socket));
        boost::shared_ptr<TProtocol> client_protocol(new TBinaryProtocol(client_transport));
        boost::shared_ptr<im::LogicInterfaceClient> client(new im::LogicInterfaceClient(client_protocol));
    
        client_transport->open();
        client->AccessToLogic(buf);
        client_transport->close();
    }
    catch(...) {
        LOG_ERROR("SendMessage Error: content=%s", buf.c_str());
    }
}
