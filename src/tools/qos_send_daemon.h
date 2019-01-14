#ifndef QOS_SEND_DAEMON_H_
#define QOS_SEND_DAEMON_H_

#include <libconfig.h++>
#include <string>
#include "logic/protocol.h"
#include "tools/database.h"

class QosSendDaemon {
public:
    QosSendDaemon();
    ~QosSendDaemon();
    bool Init(const libconfig::Setting &setting, Database *database);
    void Start();
    void Stop();
private:
    bool stop_;
    int max_retry_count_;
    int check_interval_;
    int msg_interval_;
    Database *database_;
};

#endif /*QOS_SEND_DAEMON_H_*/