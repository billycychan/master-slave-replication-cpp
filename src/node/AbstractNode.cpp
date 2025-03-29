#include "node/AbstractNode.h"
#include <iostream>

namespace replication {
namespace node {

// Thread pool implementation
AbstractNode::ThreadPool::ThreadPool(size_t num_threads) : stop(false) {
    for(size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this] {
            while(true) {
                std::function<void()> task;
                
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this] { 
                        return this->stop || !this->tasks.empty(); 
                    });
                    
                    if(this->stop && this->tasks.empty()) {
                        return;
                    }
                    
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                
                task();
            }
        });
    }
}

AbstractNode::ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    
    for(std::thread &worker : workers) {
        worker.join();
    }
}

// AbstractNode implementation
AbstractNode::AbstractNode(const std::string& id) 
    : id_(id), 
      up_(true),
      lock_(std::make_shared<std::shared_mutex>()),
      lastAppliedIndex_(0),
      replicationExecutor_(std::make_unique<ThreadPool>(5)) {
}

AbstractNode::~AbstractNode() {
    // ThreadPool cleanup happens automatically via unique_ptr
}

std::string AbstractNode::getId() const {
    return id_;
}

bool AbstractNode::isUp() const {
    return up_.load();
}

void AbstractNode::goDown() {
    std::cout << "Node " << id_ << " going DOWN" << std::endl;
    up_ = false;
}

void AbstractNode::goUp() {
    std::cout << "Node " << id_ << " coming UP" << std::endl;
    up_ = true;
}

std::string AbstractNode::read(const std::string& key) {
    if (!up_) {
        std::cout << "Node " << id_ << " is DOWN, cannot read" << std::endl;
        return "";
    }
    
    std::shared_lock<std::shared_mutex> readLock(*lock_);
    auto it = dataStore_.find(key);
    if (it != dataStore_.end()) {
        return it->second;
    }
    return "";
}

bool AbstractNode::deleteKey(const std::string& key) {
    if (!up_) {
        std::cout << "Node " << id_ << " is DOWN, cannot delete" << std::endl;
        return false;
    }
    
    std::unique_lock<std::shared_mutex> writeLock(*lock_);
    
    // Check if the key exists before attempting to delete
    auto it = dataStore_.find(key);
    if (it == dataStore_.end()) {
        std::cout << "Node " << id_ << " could not delete key '" << key << "' (not found)" << std::endl;
        return false;
    }
    
    // Remove the key from the data store
    dataStore_.erase(it);
    std::cout << "Node " << id_ << " deleted key '" << key << "'" << std::endl;
    return true;
}

std::map<std::string, std::string> AbstractNode::getDataStore() const {
    if (!up_) {
        std::cout << "Node " << id_ << " is DOWN, cannot get data store" << std::endl;
        return {};
    }
    
    std::shared_lock<std::shared_mutex> readLock(*lock_);
    return dataStore_; // This creates a copy
}

long AbstractNode::getLastLogIndex() const {
    if (!up_) {
        return -1;
    }
    return lastAppliedIndex_.load();
}

bool AbstractNode::applyLogEntry(const model::LogEntry& entry, 
                                std::shared_ptr<std::shared_mutex> lock) {
    if (!up_) {
        std::cout << "Node " << id_ << " is DOWN, cannot apply log entry" << std::endl;
        return false;
    }
    
    std::unique_lock<std::shared_mutex> writeLock(*lock);
    
    // Check if this log entry is the next in sequence
    if (entry.getId() != lastAppliedIndex_ + 1) {
        std::cout << "Node " << id_ << " received out-of-order log entry: " << entry.getId() 
                 << ", expected: " << (lastAppliedIndex_ + 1) << std::endl;
        return false;
    }
    
    // Apply the log entry to the data store based on operation type
    if (entry.isDelete()) {
        // For delete operations, remove the key from the data store
        dataStore_.erase(entry.getKey());
        std::cout << "Node " << id_ << " deleted key '" << entry.getKey() << "' from log entry" << std::endl;
    } else {
        // For write operations, put the key-value pair in the data store
        dataStore_[entry.getKey()] = entry.getValue();
        std::cout << "Node " << id_ << " wrote " << entry.getKey() << "=" 
                 << entry.getValue() << " from log entry" << std::endl;
    }
    
    // Add to log and update index
    {
        std::lock_guard<std::mutex> logLock(logMutex_);
        log_.push_back(entry);
    }
    lastAppliedIndex_ = entry.getId();
    
    std::cout << "Node " << id_ << " applied log entry: " << entry.toString() << std::endl;
    return true;
}

std::vector<model::LogEntry> AbstractNode::getLogEntriesAfter(long afterIndex) const {
    if (!up_) {
        std::cout << "Node " << id_ << " is DOWN, cannot get log entries" << std::endl;
        return {};
    }
    
    std::vector<model::LogEntry> entries;
    std::shared_lock<std::shared_mutex> readLock(*lock_);
    std::lock_guard<std::mutex> logLock(logMutex_);
    
    for (const auto& entry : log_) {
        if (entry.getId() > afterIndex) {
            entries.push_back(entry);
        }
    }
    return entries;
}

} // namespace node
} // namespace replication