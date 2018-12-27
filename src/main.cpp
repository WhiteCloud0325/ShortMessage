#include <stdio.h>
#include <iostream>
#include <signal.h>
#include <string>
#include <libconfig.h++>
#include <unistd.h>
#include "tools/logger.h"
#include "controller.h"

bool stopped = false;

static void sig_int(int sig) {
    stopped = true;
    printf("service stopping...\n");
}

int main(int argc, char* argv[]) {
    signal(SIGINT, sig_int);
    signal(SIGQUIT, sig_int);
    signal(SIGTERM, sig_int);
    
    if (argc != 2) {
        printf("the program run as: ./scheduler beam_id");
        return -1;
    }

    std::string beam_id(argv[1]);

    //日志初始化
    if (! Log.Init("./conf/log4cplus.properties")) {
        printf("Log Init error\n");
        return -1;
    }

    //配置文件初始化
    libconfig::Config config;
    config.readFile("./conf/conf.cfg");
    LOG_INFO("start read config file");

    // Controlller 初始化
    Controller controller;
    if (!controller.Init(config, beam_id)) {
        LOG_INFO("Controller Init Failed");
        return -1;
    }
    boost::thread t1(boost::bind(&Controller::ProcessEpollEvent, &controller));
    boost::thread t2(boost::bind(&Controller::ProcessMessageFromLogic, &controller));
    boost::thread t3(boost::bind(&Controller::ProcessSchedule, &controller));
    boost::thread t4(boost::bind(&Controller::ProcessMessageFromSate, &controller));
    while(!stopped) {
        usleep(1000000);
    }
    controller.Stop();
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    return 0;
}
