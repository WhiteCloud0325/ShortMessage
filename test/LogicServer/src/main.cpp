#include "LogicInterfaceHandler.h"
#include <iostream>
#include <thrift/concurrency/PlatformThreadFactory.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h> 
#include <thrift/server/TNonblockingServer.h>
#include <thrift/TProcessor.h>
#include <thrift/Thrift.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <signal.h>
using namespace im;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

bool stopped = false;

static void sig_int(int sig) {
    stopped = true;
    printf("service stopping...\n");
}

int main() {
    signal(SIGINT, sig_int);
    signal(SIGQUIT, sig_int);
    signal(SIGTERM, sig_int);

    boost::shared_ptr<ThreadManager> manager = ThreadManager::newSimpleThreadManager(10);
    boost::shared_ptr<LogicInterfaceHandler> handler(new LogicInterfaceHandler);    
    boost::shared_ptr<TProcessor> processor(new LogicInterfaceProcessor(handler));
    boost::shared_ptr<TProtocolFactory> protocol_factory(new TBinaryProtocolFactory());
    boost::shared_ptr<TServerTransport> server_transport(new TServerSocket(10000, 2 * 1000, 2 * 1000));
    boost::shared_ptr<TTransportFactory> transport_factory(new TBufferedTransportFactory());
    boost::shared_ptr<PlatformThreadFactory> factory(new PlatformThreadFactory());
    manager->threadFactory(factory);
    manager->start();
    boost::shared_ptr<apache::thrift::server::TServer>  test_server(new TThreadPoolServer(processor, server_transport, transport_factory, protocol_factory, manager));
    test_server->serve();
    while(!stopped) {
        usleep(1000000);
    }
    return 0;
}
