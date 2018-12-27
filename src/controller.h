#ifndef controller_h_
#define controller_h_

#include <libconfig.h++>
#include <unordered_map>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include "commons/tcp_socket.h"
#include "commons/epoll_wrapper.h"
#include "commons/fifo_queue.h"
#include "tools/priority_queue.h"
#include <thrift/server/TServer.h>

class LogicInterfaceHandler;
class Controller {
public:
    Controller();
    ~Controller();
    bool Init(const libconfig::Config &config, const std::string beam_id);
    void Start();
    void Stop();
    friend class LogicInterfaceHandler;
    /**
     *  Process Tcp Connetion and receiver message from TcpClient
     */ 
    void ProcessEpollEvent();

    /**
     *  Get Message from priortyqueue and Send message to tcpclient
     */ 
    void ProcessSchedule();

    /**
     *  Get string from fifoqueue and Send to Logic Service
     */ 
    void ProcessMessageFromSate();

    /**
     * Get string from Logic
     */ 
    void ProcessMessageFromLogic();

private:
    void HandleEvent(int socket_fd);
    void AddObservedClient(int socket_fd);
    void DelObservedClient(int socket_fd);
private:
    bool            stop_;
    std::string     logic_ip_;
    int             logic_port_;
    int             write_timeout_;
    int             read_timeout_;
    int             thread_cnt_; 
    int             beam_id_;
    int             tcp_port_;
    int             thrift_port_;
    
    boost::shared_ptr<TcpSocket> server_socket_;
    boost::shared_ptr<TcpSocket> client_socket_;
    Epoll epoll_;
    FifoQueue fifo_queue_;
    PriorityQueue priority_queue_;
    boost::shared_ptr<apache::thrift::server::TServer>  server_;
    boost::shared_mutex client_mutex_;
};

#endif /* define control_h_*/
