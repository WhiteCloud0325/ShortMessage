#include "im.pb.h"
#include "LogicInterface.h"
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <string>
#include <thrift/Thrift.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
using namespace apache;
using namespace apache::thrift;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;

int main() {
    im::UserRegisterRequest user_register_request;
    user_register_request.set_number("15620623833");
    user_register_request.set_password("123456");
    user_register_request.set_nickname("wei");
    user_register_request.set_ip("162.105.85.45");
    std::string request;
    user_register_request.SerializeToString(&request);
    std::string server_ip = "162.105.85.118";
    int port = 9090;
    boost::shared_ptr<TSocket> client_socket(new TSocket(server_ip, port));
    boost::shared_ptr<TTransport> client_transport(new TBufferedTransport(client_socket));
    boost::shared_ptr<TProtocol> client_protocol(new TBinaryProtocol(client_transport));
    boost::shared_ptr<im::LogicInterfaceClient> client(new im::LogicInterfaceClient(client_protocol));
    im::Request client_request;
    client_request.__set_type(im::REGISTER_REQUEST);
    client_request.__set_content(request);
    client_transport->open();
    client->SendMessage(client_request);
    client_transport->close();
    return 0;
}
