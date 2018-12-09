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
    while(!stopped) {
        usleep(1000000);
    }
    controller.Stop();
    run_thread.join();
    return 0;
}