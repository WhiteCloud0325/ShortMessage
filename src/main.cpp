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

    // Controlller 初始化
    Controller controller;
    if (!controller.Init(config)) {
        LOG_INFO("Controller Init Failed");
        return -1;
    }

    while(!stopped) {
        usleep(1000000);
    }

    return 0;
}
