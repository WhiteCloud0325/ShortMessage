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
    LOG_INFO("Controller Init Success");
    return true;
}

void Controller::Run() {
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
}
