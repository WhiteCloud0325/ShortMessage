#ifndef controller_h_
#define controller_h_

#include <libconfig.h++>
#include <unordered_map>
#include <boost/thread/thread.hpp>
#include "commons/tcp_socket.h"
#include "commons/epoll_wrapper.h"
#include "commons/fifo_queue.h"


class Controller {
public:
    Controller();
    ~Controller();
    bool Init(const libconfig::Config &config);
    void Start();
    void Stop();
private:
    void AddObservedClient(int socket_fd);
    void DelObservedClient(int socket_fd);
    void HandleEvent(int socket_fd);
    void ProcessMessage();
private:
    bool stop_;
    int access_port_;
    int thread_num_;
    TcpSocket server_socket_;
    Epoll epoll_;
    FifoQueue fifo_queue_;
    std::unordered_map<int, CommonSocket*> observed_client_;
    boost::thread_group group_;
};

#endif /* define control_h_*/
