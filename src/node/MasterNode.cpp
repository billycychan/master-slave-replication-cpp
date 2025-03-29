#include "node/MasterNode.h"
#include "node/SlaveNode.h"
#include <iostream>

namespace replication {
namespace node {

MasterNode::MasterNode(const std::string& id)
    : AbstractNode(id),
      nextLogId_(1) {
}

MasterNode::~MasterNode() {
    shutdown();
}

void MasterNode::registerSlave(std::shared_ptr<SlaveNode> slave) {
    std::lock_guard<std::mutex> guard(slavesMutex_);
    slaves_.insert(slave);
    std::cout << "Master " << id_ << " registered slave: " << slave->getId() << std::endl;
}

bool MasterNode::write(const std::string& key, const std::string& value) {
    if (!up_) {
        std::cout << "Master " << id_ << " is DOWN, cannot write" << std::endl;
        return false;
    }

    std::unique_lock<std::shared_mutex> writeLock(*lock_);
    
    // Create a new log entry for write operation
    model::LogEntry entry(nextLogId_++, key, value, model::LogEntry::OperationType::WRITE);
    
    // Apply to the master's data store first
    dataStore_[key] = value;
    
    // Add to log
    {
        std::lock_guard<std::mutex> logLock(logMutex_);
        log_.push_back(entry);
    }
    
    lastAppliedIndex_ = entry.getId();
    
    std::cout << "Master " << id_ << " wrote " << key << "=" << value 
              << " (Log ID: " << entry.getId() << ")" << std::endl;
    
    // Track replication status
    {
        std::lock_guard<std::mutex> pendingLock(pendingReplicationsMutex_);
        pendingReplications_[entry.getId()] = std::set<std::string>();
    }
    
    // Asynchronously replicate to slaves
    replicateToSlaves(entry);
    
    return true;
}

bool MasterNode::deleteKey(const std::string& key) {
    if (!up_) {
        std::cout << "Master " << id_ << " is DOWN, cannot delete" << std::endl;
        return false;
    }

    std::unique_lock<std::shared_mutex> writeLock(*lock_);
    
    // Check if the key exists before attempting to delete
    auto it = dataStore_.find(key);
    if (it == dataStore_.end()) {
        std::cout << "Master " << id_ << " could not delete key '" << key << "' (not found)" << std::endl;
        return false;
    }
    
    // Create a new log entry for delete operation
    model::LogEntry entry(nextLogId_++, key, "", model::LogEntry::OperationType::DELETE);
    
    // Remove the key from the data store
    dataStore_.erase(it);
    
    // Add to log
    {
        std::lock_guard<std::mutex> logLock(logMutex_);
        log_.push_back(entry);
    }
    
    lastAppliedIndex_ = entry.getId();
    
    std::cout << "Master " << id_ << " deleted key '" << key 
              << "' (Log ID: " << entry.getId() << ")" << std::endl;
    
    // Track replication status
    {
        std::lock_guard<std::mutex> pendingLock(pendingReplicationsMutex_);
        pendingReplications_[entry.getId()] = std::set<std::string>();
    }
    
    // Asynchronously replicate to slaves
    replicateToSlaves(entry);
    
    return true;
}

void MasterNode::replicateToSlaves(const model::LogEntry& entry) {
    std::vector<std::shared_ptr<SlaveNode>> currentSlaves;
    
    {
        std::lock_guard<std::mutex> guard(slavesMutex_);
        for (const auto& slave : slaves_) {
            currentSlaves.push_back(slave);
        }
    }

    for (const auto& slave : currentSlaves) {
        replicationExecutor_->enqueue([this, slave, entry]() {
            if (slave->isUp()) {
                bool success = slave->applyLogEntry(entry, lock_);
                if (success) {
                    // Track successful replication
                    std::lock_guard<std::mutex> pendingLock(pendingReplicationsMutex_);
                    auto it = pendingReplications_.find(entry.getId());
                    if (it != pendingReplications_.end()) {
                        it->second.insert(slave->getId());
                        std::cout << "Master " << id_ << " replicated log entry " << entry.getId() 
                                  << " to slave " << slave->getId() << std::endl;
                    }
                } else {
                    slave->recoverSlave();
                }
            } else {
                std::cout << "Master " << id_ << " couldn't replicate to slave " 
                          << slave->getId() << " (DOWN)" << std::endl;
            }
        });
    }
}

void MasterNode::shutdown() {
    // Thread pool is cleaned up in the AbstractNode destructor
}

} // namespace node
} // namespace replication