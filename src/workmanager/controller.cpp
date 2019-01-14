#include <thrift/concurrency/PlatformThreadFactory.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h> 
#include <thrift/server/TNonblockingServer.h>
#include <thrift/TProcessor.h>
#include <thrift/Thrift.h>
#include "workmanager/controller.h"
#include "tools/logger.h"
#include "workmanager/LogicInterfaceHandler.h"
#include "workmanager/send_helper.h"

using namespace im;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

Controller::Controller():port_(0),
                         write_timeout_(0), 
                         read_timeout_(0),
                         thread_cnt_(0),
                         stop_(false) {
}

Controller::~Controller() {
    if (!stop_)
        Stop();
}

bool Controller::Init(libconfig::Config &config) {
    try {
        libconfig::Setting &db_setting = config.lookup("Database");
        if (!database_.Init(db_setting)) {
            return false;
        }

        libconfig::Setting &send_helper_setting = config.lookup("SendHelper");
        if (! SendHelper::GetInstance()->Init(send_helper_setting)) {
            LOG_INFO("SendHelper Init Failed");
            return false;
        }

        libconfig::Setting &qos_setting = config.lookup("Qos");
        if (!qos_send_daemon_.Init(qos_setting, &database_)) {
            LOG_INFO("QosSendDaemon Init Failed");
            return false;
        }
        
        libconfig::Setting &server_setting = config.lookup("ThriftServer");
        if (!server_setting.lookupValue("port", port_)) {
            return false;
        }

        if (!server_setting.lookupValue("write_timeout", write_timeout_)) {
            return false;
        }

        if (!server_setting.lookupValue("read_timeout", read_timeout_)) {
            return false;
        }
        
        if (!server_setting.lookupValue("thread_cnt", thread_cnt_)) {
            return false;
        }
    }
    catch(...) {
        LOG_INFO("Controller Init Exception Failed");
        return false;
    }
    user_manager_.Init(&database_);
    message_manager_.Init(&database_);
    mobile_manager_.Init(&database_);
    LOG_INFO("Controller Init Success");
    return true;
}

void Controller::Run() {
    for (int i = 0; i < thread_cnt_; ++i) {
        group_.create_thread((boost::bind(&Controller::Execute, this));
    } 
    boost::shared_ptr<apache::thrift::concurrency::ThreadManager> manager;
    try {
        boost::shared_ptr<TProtocolFactory> protocol_factory(new TBinaryProtocolFactory());
        boost::shared_ptr<LogicInterfaceHandler> handler(new LogicInterfaceHandler(this));
        boost::shared_ptr<TProcessor> processor(new LogicInterfaceProcessor(handler));
        boost::shared_ptr<TServerTransport> server_transport(new TServerSocket(port_, write_timeout_ * 1000, read_timeout_ * 1000));
        boost::shared_ptr<TTransportFactory> transport_factory(new TBufferedTransportFactory());
        manager = ThreadManager::newSimpleThreadManager(thread_cnt_);
        boost::shared_ptr<PlatformThreadFactory> factory(new PlatformThreadFactory());
        manager->threadFactory(factory);
        manager->start();
        //server_.reset(new TNonblockingServer(processor, protocol_factory, port_, manager));
        server_.reset(new TThreadPoolServer(processor, server_transport, transport_factory, protocol_factory, manager));
    }
    catch(...) {
        return;
    }
    server_->serve();
}

void Controller::Stop() {
    stop_ = true;
    server_->stop();
    qos_send_daemon_.Stop();
    group_.join_all();
}

bool Controller::Push(const std::string &str) {
    boost::unique_lock<boost::shared_mutex> ulock(mutex_);
    fifo_queue_.push(str);
    return true;
}

bool Controller::Pop(std::string &str) {
    boost::unique_lock<boost::shared_mutex> ulock(mutex_);
    if (fifo_queue_.empty()) {
        return false;
    }
    str = fifo_queue_.front();
    fifo_queue_.pop();
    return true;
}

void Controller::Execute() {
    Connetcion_T conn = NULL;
    while (!stop_) {
        if (conn == NULL) {
            conn = database_->GetConnection();
        }
        if (conn == NULL) {
            sleep(1);
            continue;
        }
        if (Connection_ping(conn)) {
            std::string request;
            if (!Pop(str)) {
                continue;
            }
            timeval start;
            gettimeofday(&start, NULL);
            ControlHead* control_head = (ControlHead*)request.c_str();
            switch (control_head->type) {
                case Login:
                    controller_->mobile_manager_.ProcessLogin(control_head, conn);
                    break;
                case Logout:
                    controller_->mobile_manager_.ProcessLogout(control_head, conn);
                    break; 
                case MOBILE_MANAGER_REQUEST:
                    controller_->mobile_manager_.ProcessMobileRequest(control_head, conn);
                    break;
                case SIMPLE_MESSAGE:
                    controller_->message_manager_.ProcessSimpleMessage(control_head, conn);
                    break;
                case FORWARD_NOACK_MESSAGE:
                    controller_->message_manager_.ProcessForwardNoAckMessage(control_head, conn);
                    break;
                case BACKWARD_NOACK_MESSAGE:
                    controller_->message_manager_.ProcessBackwardNoAckMessage(control_head, conn);
                    break;
                case COMPLETE_MESSAGE:
                    controller_->message_manager_.ProcessCompleteMessage(control_head, conn);
                    break;
                case INQUIRE_MESSAGE_REQUEST:
                    break;
                case RECEIPTE:
                    controller_->message_manager_.ProcessReceipt(control_head, conn);
                    break;
                default:
                    break;
            }
            timeval end;
            gettimeofday(&end, NULL);
            uint32_t cost_time = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
            LOG_DEBUG("AccessToLogic: time=%ld", cost_time);
        }
        else {
            Connection_close(conn);
            conn = NULL;
        }
    }
}
