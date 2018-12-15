#ifndef QOS_SEND_DAEMON_H_
#define QOS_SEND_DAEMON_H_

#include <libconfig.h++>
#include <string>
#include <tbb/concurrent_hash_map.h>
#include "tools/protocol.h"

typedef tbb::concurrent_hash_map<int64_t, Protocol> msg_hash_map;
class QosSendDaemon {
public:
    QosSendDaemon();
    ~QosSendDaemon();
    bool Init(const libconfig::Setting &setting);
    void Start();
    void Stop();
    bool Put(Protocol &p);
    bool Remove(int64_t key);
private:
    bool stop_;
    int max_retry_count_;
    int check_interval_;
    int msg_interval_;
    msg_hash_map send_messages_;
};

#endif /*QOS_SEND_DAEMON_H_*/