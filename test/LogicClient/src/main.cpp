#include "LogicInterface.h"
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <string>
#include <thrift/Thrift.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include "logic/protocol.h"
#include <arpa/inet.h>
#include <string.h>
using namespace apache;
using namespace apache::thrift;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;

int main() {
    uint32_t from_id = 1;
    uint32_t to_id = 2;
    uint32_t frame_id = 3;
    uint8_t  type = 0x40;
    uint8_t  retain = 0x00;
    char content[] = "bbbbbb";
    char buf[1024] = {0};
    char *pos = buf;
    *(uint32_t*)pos = htonl(to_id);
    pos+=4;
    *(uint32_t*)pos = htonl(from_id);
    pos += 4;
    *(uint16_t*)pos = htons(frame_id);
    pos +=2;
    *(uint8_t*)pos++ = type;
    *(uint8_t*)pos++ = retain;
    strcpy(pos, content);
    std::string request(buf);
    int port = 9090;
    std::string server_ip = "162.105.85.118";
    boost::shared_ptr<TSocket> client_socket(new TSocket(server_ip, port));
    boost::shared_ptr<TTransport> client_transport(new TBufferedTransport(client_socket));
    boost::shared_ptr<TProtocol> client_protocol(new TBinaryProtocol(client_transport));
    boost::shared_ptr<im::LogicInterfaceClient> client(new im::LogicInterfaceClient(client_protocol));
    client_transport->open();
    client->AccessToLogic(request);
    client_transport->close();
    return 0;
}
