#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <libconfig.h++>
#include <boost/shared_ptr.hpp>
#include <thrift/server/TServer.h>
#include "tools/logger.h"
#include "tools/database.h"
#include "logic/user_manager.h"
#include "logic/mobile_manager.h"
#include "tools/qos_send_daemon.h"

class Controller {
public:
    Controller();
    ~Controller();
    bool Init(libconfig::Config &config);
    void Run();
    void Stop();
public:
    UserManager user_manager_;
    MessageManager message_manager_;
    QosSendDaemon qos_send_daemon_;
private:
    boost::shared_ptr<apache::thrift::server::TServer>  server_;
    Database database_;
    int port_;
    int write_timeout_;
    int read_timeout_;
    int thread_cnt_;
    bool stop_;
};

#endif /* define controller_h_*/
