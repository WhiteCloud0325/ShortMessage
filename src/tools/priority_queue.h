#ifndef priority_queue_h_
#define priority_queue_h_

#include <queue>
#include <string>
#include <vector>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#define INIT_QUEUE_SIZE 10000

struct QueueItem {
    QueueItem(){}
    QueueItem(int level_, const char* ptr):level(level_), message(ptr){}
    QueueItem(int level_, const std::string &ptr):level(level_), message(ptr){}
    int level;
    std::string message;
};

struct Compare {
    bool operator() (const QueueItem &lhs, const QueueItem &rhs) {
        if (lhs.level < rhs.level) {
            return true;
        }
        else {
            return false;
        }
    }
};

class PriorityQueue {
public:
    PriorityQueue(int size = INIT_QUEUE_SIZE);
    ~PriorityQueue();
    bool Push(int level, const std::string &str);
    bool Pop(std::string &str);
private:
    int size_;
    std::priority_queue<QueueItem, std::vector<QueueItem>, Compare> queue_;
    boost::condition condition_;
    boost::mutex mutex_lock_;
};


#endif /* define priority_queue_h_*/