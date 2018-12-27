#include "tools/priority_queue.h"
#include "tools/logger.h"

PriorityQueue::PriorityQueue(int size):size_(size) {
}

PriorityQueue::~PriorityQueue() {

}

bool PriorityQueue::Push(int level, const std::string &str) {
    boost::unique_lock<boost::mutex> ulock(mutex_lock_);
    if ( size_ == queue_.size()) {
        LOG_INFO("PriorityQueue push data failed: too many message");
        return false;
    }
    queue_.push(QueueItem(level, str));
    return true;
}

bool PriorityQueue::Pop(std::string &str) {
    boost::unique_lock<boost::mutex> ulock(mutex_lock_);
    if (queue_.empty()) {
        return false;
    }
    str = queue_.top();
    queue_.pop();
    return true;
}