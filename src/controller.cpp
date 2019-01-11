#include "controller.h"
#include "tools/logger.h"
#include "tools/send_helper.h"
#include <string.h>
#include <boost/bind.hpp>

Controller::Controller():access_port_(0),
                         stop_(false) {

}

Controller::~Controller(){
    //delete all pointers in map
    while(! observed_client_.empty()){
        std::unordered_map<int, CommonSocket*>::iterator it = observed_client_.begin();
        delete it->second;
        observed_client_.erase(it);
    }
}

bool Controller::Init(const libconfig::Config &config) {
    stop_ = false;
    try {
        libconfig::Setting &access_setting = config.lookup("AccessServer");
        libconfig::Setting &sender_setting = config.lookup("LogicServer");

        if(!access_setting.lookupValue("port", access_port_)) {
            return false;
        }

        if (!access_setting.lookupValue("thread_num", thread_num_)) {
            return false;
        }
        
        if(! SendHelper::GetInstance()->Init(sender_setting)) {
            return false;
        }
    }
    catch(...) {
        LOG_INFO("Controller Init Exception Failed");
        return false;
    }

    if (!server_socket_.Create()) {
        return false;
    }
    if (!server_socket_.SetBlocking(false)) {
        return false;
    }
    if (!server_socket_.Bind("0.0.0.0", access_port_)) {
        return false;
    }
    if (!server_socket_.Listen(20)) {
        return false;
    }
    epoll_.AddEvent(server_socket_.socket_fd(), EPOLLIN);
    observed_client_.insert(std::pair<int, CommonSocket*>(server_socket_.socket_fd(), &server_socket_));
    LOG_INFO("Controller Init Success");
    return true;

}

void Controller::Start() {
    for (int i = 0; i < thread_num_; ++i) {
        group_.create_thread(boost::bind(&Controller::ProcessMessage, this));
    }
    while (!stop_) {
        int event_num = epoll_.Wait();
        const struct epoll_event* ready_events = epoll_.ReadyEvents();
        //handle events
        for (int i = 0; i < event_num; ++i) {
            HandleEvent(ready_events[i].data.fd);
        }
    }
}

void Controller::Stop() {
    stop_= true;
    group_.join_all();
}

void Controller::AddObservedClient(int socket_fd) {
    TcpSocket* client = new TcpSocket;
    client->Create(socket_fd);  //build socket by using socket_fd
	client->SetBlocking(false); //set socket in no block mode
    if (! epoll_.AddEvent(socket_fd, EPOLLIN)){ //add socket fd to epoll events
        LOG_FATAL("Controller AddObserverClient Failed:epoll add event failed");
    }
    observed_client_.insert(std::pair<int, CommonSocket*>(socket_fd, client));
}

void Controller::DelObservedClient(int socket_fd) {
    std::unordered_map<int, CommonSocket*>::iterator it = observed_client_.find(socket_fd);
    if (it == observed_client_.end()) {
        return;
    }
    delete it->second;
    if (! epoll_.DelEvent(socket_fd, EPOLLIN)){ //add socket fd to epoll events
        LOG_FATAL("Controller DelObservedClient Failed: epoll del event failed");
    }
}

void Controller::HandleEvent(int socket_fd) {
    if (socket_fd == server_socket_.socket_fd()) { //server listen fd event
        int conn_fd;
        struct sockaddr_in client_addr;
        if (! server_socket_.Accept(&conn_fd, &client_addr)) {
            LOG_FATAL("Access Server Accept Connection Failed");
            return;
        }
        LOG_DEBUG("Access Server Accept a new Connection: fd=%d", conn_fd);
        //TODO
        AddObservedClient(conn_fd);
    }
    else {
        const int kBufSize = 50000, kHeadLen = 4;
        int recved_len;
        char recv_buffer[kBufSize];
        TcpSocket* client = dynamic_cast<TcpSocket*>(observed_client_[socket_fd]);
        int complete_packet_num = client->RecvPacket(recv_buffer, &recved_len);
        if (complete_packet_num < 0){ //receive failed or not receive 1 or more complete packet
            LOG_INFO("Controller HandleEvent Client Connection Closed: fd=%d", socket_fd);
            DelObservedClient(socket_fd);
        }
        else{
            //std::cout << "package num: " << complete_packet_num << std::endl; 
            char* read_pos = recv_buffer;
            for (int i = 0; i < complete_packet_num; i++){//get all complete packets in buffer
                int packet_len = *(int*)read_pos;
                read_pos += kHeadLen;
                //TODO
                fifo_queue_.PushPacket(read_pos, packet_len);
                read_pos += packet_len;
            }
        }
    }
}

void Controller::ProcessMessage() {
    char buf[1024];
    uint16_t len = 0;
    while(!stop_) {
        memset(buf, 0 ,1024);
        len = 0;
        fifo_queue_.PopPacket(buf, &len);
        /*if (len >= 40) {
            char *pos = buf;
            uint32_t to_id = *(uint32_t*)pos;
            pos += 28;
            *(uint32_t*)pos = to_id;
            std::string message(pos, len - 28);
            SendHelper::GetInstance()->SendMessage(message);
        }*/
        /*for (int i = 0; i < len; ++i) {
            printf("%02x ", (uint8_t)*(buf+i));
        }
        printf("\n");*/
        std::string message(buf, len);
        SendHelper::GetInstance()->SendMessage(message);
    }
}
