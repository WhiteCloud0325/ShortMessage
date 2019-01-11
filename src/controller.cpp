#include <thrift/concurrency/PlatformThreadFactory.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h> 
#include <thrift/server/TNonblockingServer.h>
#include <thrift/TProcessor.h>
#include <thrift/Thrift.h>
#include "controller.h"
#include "tools/logger.h"
#include <string.h>
#include <boost/bind.hpp>
#include "LogicInterfaceHandler.h"
using namespace im;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

Controller::Controller(): stop_(false),
                          write_timeout_(100),
                          read_timeout_(100),
                          thread_cnt_(20),
                          beam_id_(0),
                          tcp_port_(0),
                          thrift_port_(0){

}

Controller::~Controller(){
    if (!stop_) {
        Stop();
    }
}

bool Controller::Init(const libconfig::Config &config, const std::string beam_id) {
    stop_ = false;
    try {
        const libconfig::Setting &common_setting = config.lookup("Common");
        const libconfig::Setting &beam_setting = config.lookup(beam_id);
        if (!common_setting.lookupValue("write_timeout", write_timeout_)) {
            return false;
        }
        if (!common_setting.lookupValue("read_timeout", read_timeout_)) {
            return false;
        }
        if (!common_setting.lookupValue("thread_cnt", thread_cnt_)) {
            return false;
        }
        if (!beam_setting.lookupValue("beam_id", beam_id_)) {
            return false;
        }
        if (!beam_setting.lookupValue("thrift_port", thrift_port_)) {
            return false;
        }
        if (!beam_setting.lookupValue("tcp_port", tcp_port_)) {
            return false;
        }
    }
    catch(...) {
        LOG_INFO("Controller Init Exception Failed");
        return false;
    }
    // tcp server Init
    server_socket_.reset(new TcpSocket);
    if(!server_socket_->Create()) {
        return false;
    }
    if (!server_socket_->SetBlocking(false)) {
        return false;
    }
    if (!server_socket_->Bind("0.0.0.0", tcp_port_)) {
        return false;
    }
    if (!server_socket_->Listen(5)) {
        return false;
    }
    epoll_.AddEvent(server_socket_->socket_fd(), EPOLLIN);

    //thrift server Init
    boost::shared_ptr<apache::thrift::concurrency::ThreadManager> manager;
    try {
        boost::shared_ptr<TProtocolFactory> protocol_factory(new TBinaryProtocolFactory());
        boost::shared_ptr<LogicInterfaceHandler> handler(new LogicInterfaceHandler(this));
        boost::shared_ptr<TProcessor> processor(new LogicInterfaceProcessor(handler));
        boost::shared_ptr<TServerTransport> server_transport(new TServerSocket(thrift_port_, write_timeout_ * 1000, read_timeout_ * 1000));
        boost::shared_ptr<TTransportFactory> transport_factory(new TBufferedTransportFactory());
        manager = ThreadManager::newSimpleThreadManager(thread_cnt_);
        boost::shared_ptr<PlatformThreadFactory> factory(new PlatformThreadFactory());
        manager->threadFactory(factory);
        manager->start();
        //server_.reset(new TNonblockingServer(processor, protocol_factory, port_, manager));
        server_.reset(new TThreadPoolServer(processor, server_transport, transport_factory, protocol_factory, manager));
    }
    catch(...) {
        LOG_INFO("Controller Init Failed: thrift server init exception");
        return false;
    }
    LOG_INFO("Controller Init Success");
    return true;

}

void Controller::Start() {
   
}

void Controller::Stop() {
    stop_ = true;
    server_->stop();
}

void Controller::ProcessEpollEvent() {
    while (!stop_) {
        int event_num = epoll_.Wait();
        const struct epoll_event* ready_events = epoll_.ReadyEvents();
        //handle events
        for (int i = 0; i < event_num; ++i) {
            HandleEvent(ready_events[i].data.fd);
        }
    }
}

void Controller::HandleEvent(int socket_fd) {
    if (socket_fd == server_socket_->socket_fd()) { //server listen fd event
        int conn_fd;
        struct sockaddr_in client_addr;
        if (! server_socket_->Accept(&conn_fd, &client_addr)) {
            LOG_FATAL("Access Server Accept Connection Failed");
            return;
        }
        LOG_DEBUG("Access Server Accept a new Connection: fd=%d", conn_fd);
        //TODO
        AddObservedClient(conn_fd);
    }
    else if (NULL != client_socket_ && socket_fd == client_socket_->socket_fd()) {
        const int kBufSize = 50000, kHeadLen = 4;
        int recved_len;
        char recv_buffer[kBufSize];
        int complete_packet_num = client_socket_->RecvPacket(recv_buffer, &recved_len);
        if (complete_packet_num < 0){ //receive failed or not receive 1 or more complete packet
            LOG_INFO("Controller HandleEvent Client Connection Closed: fd=%d", socket_fd);
            DelObservedClient(socket_fd);
        }
    }
}

void Controller::AddObservedClient(int socket_fd) {
    boost::unique_lock<boost::shared_mutex> lock(client_mutex_);
    if (client_socket_!= NULL) {
        epoll_.DelEvent(client_socket_->socket_fd(), EPOLLIN);
    }
    client_socket_.reset(new TcpSocket);
    client_socket_->Create(socket_fd);  //build socket by using socket_fd
	client_socket_->SetBlocking(false); //set socket in no block mode
    if (! epoll_.AddEvent(socket_fd, EPOLLIN)){ //add socket fd to epoll events
        LOG_FATAL("Controller AddObserverClient Failed:epoll add event failed");
    }
}

void Controller::DelObservedClient(int socket_fd) {
    boost::unique_lock<boost::shared_mutex> lock(client_mutex_);
    client_socket_.reset();
    if (! epoll_.DelEvent(socket_fd, EPOLLIN)){ //delete socket fd to epoll events
        LOG_FATAL("Controller DelObservedClient Failed: epoll del event failed");
    }
}

void Controller::ProcessSchedule() {
    while (!stop_) {
        std::string msg;
        {
            boost::unique_lock<boost::shared_mutex> lock(client_mutex_);
            if (client_socket_ != NULL) {
                if (priority_queue_.Pop(msg)) {
                    for (int i = 0; i < msg.size(); ++i) {
                        printf("%02x ", (uint8_t)msg[i]);
                    }
                    printf("\n");
                    client_socket_->SendPacket(const_cast<char*>(msg.c_str()), msg.size());
                    LOG_DEBUG("Schedule Send Message")
                }
            }
        } 
        //TODO 定时
    }
}

void Controller::ProcessMessageFromLogic() {
    server_->serve();
}



