#include "workmanager/send_helper.h"
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
        if (!setting.lookupValue("port", access_port_)) {
            return false;
        }
        if (!setting.lookupValue("ip", access_ip_)) {
            return false;
        }
    }
    catch (...) {
        LOG_INFO("SendHelper Init Exception");
        return false;
    }
    return true;
}

void SendHelper::SendMessageData(Protocol &p) {
    boost::shared_ptr<TSocket> client_socket(new TSocket(access_ip_, access_ip_));
    boost::shared_ptr<TTransport> client_transport(new TBufferedTransport(client_socket));
    boost::shared_ptr<TProtocol> client_protocol(new TBinaryProtocol(client_transport));
    boost::shared_ptr<im::LogicInterfaceClient> client(new im::LogicInterfaceClient(client_protocol));

    im::Response send;
    send.__set_type(p->type);
    send.__set_udi(p->uid);
    send.__set_content(p->data_content);
    send.__set_ip(p->ip);
    client_transport->open();
    client->AckMessage(send);
    client_transport->close();
}

void SendHelper::SendData() {
    
}