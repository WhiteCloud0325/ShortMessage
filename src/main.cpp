#include <stdio.h>
#include <iostream>
#include <string>
#include <signal.h>
#include <libconfig.h++>
#include <zdb.h>
#include <unistd.h>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "tools/database.h"
#include "tools/logger.h"
#include "workmanager/controller.h"
#include "workmanager/send_helper.h"

#include <thrift/Thrift.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
using namespace apache;
using namespace apache::thrift;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;

bool stopped = false;

static void sig_int(int sig) {
    stopped = true;
    printf("service stopping...\n");
}
int main() {
    signal(SIGINT, sig_int);
    signal(SIGQUIT, sig_int);
    signal(SIGTERM, sig_int);
    
    //日志初始化
    if (! Log.Init("./conf/log4cplus.properties")) {
        printf("Log Init error\n");
        return -1;
    }

    //配置文件初始化
    libconfig::Config config;
    config.readFile("./conf/conf.cfg");
    LOG_INFO("start read config file");
    
    // 构造controller并初始化
    Controller controller;
    if (!controller.Init(config)) {
        return -1;
    }
    boost::thread run_thread(boost::bind(&Controller::Run, &controller)); 
    boost::thread qos_thread(boost::bind(&QosSendDaemon::Start, &(controller.qos_send_daemon_)));
    while(!stopped) {
        usleep(1000000);
    }
    controller.Stop();
    run_thread.join();
    qos_thread.join();
    /*
    uint32_t from_id = 1;
    uint32_t to_id = 2;
    uint16_t frame_id = 3;
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
    pos += strlen(content) + 1;
    std::string request(buf, pos - buf);
    int port = 10000;
    std::string server_ip = "162.105.85.118";
    im::AccessMessage message;
    message.__set_uid(to_id);
    std::vector<int32_t> beams = {1, 3};
    message.__set_beam_id(beams);
    message.__set_content(request);
    boost::shared_ptr<TSocket> client_socket(new TSocket(server_ip, port));
    boost::shared_ptr<TTransport> client_transport(new TBufferedTransport(client_socket));
    boost::shared_ptr<TProtocol> client_protocol(new TBinaryProtocol(client_transport));
    boost::shared_ptr<im::LogicInterfaceClient> client(new im::LogicInterfaceClient(client_protocol));
    std::cout << request.size() << std::endl;
    client_transport->open();
    client->LogicToAccess(message);
    //client->AccessToLogic(request);
    client_transport->close();
    */
    return 0;
}
