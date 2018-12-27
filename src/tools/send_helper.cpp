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

bool SendHelper::Init(const std::string &ip, const int port) {
    logic_ip_ = ip;
    logic_port_ = port;
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
