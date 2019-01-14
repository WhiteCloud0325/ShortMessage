#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <libconfig.h++>
#include <boost/shared_ptr.hpp>
#include <thrift/server/TServer.h>
#include <boost/thread.hpp>
#include <queue>
#include <string>
#include <unistd.h>
#include "tools/logger.h"
#include "tools/database.h"
#include "logic/user_manager.h"
#include "logic/mobile_manager.h"
#include "logic/message_manager.h"
#include "tools/qos_send_daemon.h"

class Controller {
public:
    Controller();
    ~Controller();
    bool Init(libconfig::Config &config);
    void Run();
    void Stop();
    void Execute();
    bool Push(const std::string &str);
    bool Pop(std::string &str);
public:
    QosSendDaemon qos_send_daemon_;
private:
    UserManager user_manager_;
    MessageManager message_manager_;
    MobileManager mobile_manager_;
private:
    boost::shared_ptr<apache::thrift::server::TServer>  server_;
    Database database_;
    int port_;
    int write_timeout_;
    int read_timeout_;
    int thread_cnt_;
    bool stop_;
    boost::shared_mutex mutex_;
    std::queue<std::string> fifo_queue_;
    boost::thread_group group_;
};

#endif /* define controller_h_*/
