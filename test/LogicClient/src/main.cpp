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
#pragma pack(1)
int main() {
    uint32_t from_id = 1;
    uint32_t to_id = 2;
    uint16_t frame_id = 3;
    uint8_t  type = 0x60;
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
    pos += strlen(content) + 1;
    std::string request(buf, pos - buf);


    //mobile manage
    /*char buf[1024] = {0};
    char *pos = buf;
    *(uint32_t*)pos = htonl(0);
    pos += 4;
    *(uint32_t*)pos = htonl(1);
    pos += 4;
    *(uint16_t*)pos = htons(5);
    pos += 2;
    *(uint8_t*)pos++ = 0x00;
    *(uint8_t*)pos++ = 0x00;
    *(uint32_t*)pos = time(NULL);
    pos += 4;
    *(uint8_t*)pos++ = 0x10;
    *(float*)pos = 100.1;
    pos += 4;
    *(float*)pos = 32.8;
    pos += 4;
    *(float*)pos = 1000.2;
    pos += 4;
    *(uint8_t*)pos++ = 0x00;
    *(uint8_t*)pos++ = 2;
    *(uint8_t*)pos++ = 1;
    *(uint8_t*)pos++ = 2;
    *(float*)pos = 4.3;
    pos += 4;
    *(uint8_t*)pos++ = 5;
    *(uint8_t*)pos++ = 10;
    *(float*)pos = 3.3;
    pos += 4;
    std::string str(buf, (pos - buf) + 1);
    */    

    int port = 9090;
    std::string server_ip = "162.105.85.118";
    /*im::AccessMessage message;
    message.__set_uid(to_id);
    std::vector<int32_t> beams = {1, 3};
    message.__set_beam_id(beams);
    message.__set_content(request);*/
    boost::shared_ptr<TSocket> client_socket(new TSocket(server_ip, port));
    boost::shared_ptr<TTransport> client_transport(new TBufferedTransport(client_socket));
    boost::shared_ptr<TProtocol> client_protocol(new TBinaryProtocol(client_transport));
    boost::shared_ptr<im::LogicInterfaceClient> client(new im::LogicInterfaceClient(client_protocol));
    client_transport->open();
    //client->LogicToAccess(message);
    client->AccessToLogic(request);
    client_transport->close();
    return 0;
}
