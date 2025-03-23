// src/node/AbstractNode.cpp
#include "node/AbstractNode.h"

AbstractNode::AbstractNode(const std::string& id)
    : id(id), up(true), lastAppliedIndex(0) {
}

AbstractNode::~AbstractNode() {
    // Wait for all pending replication tasks to complete
    for (auto& task : replicationTasks) {
        if (task.valid()) {
            task.wait();
        }
    }
}

std::string AbstractNode::getId() const {
    return id;
}

bool AbstractNode::isUp() const {
    return up.load();
}

void AbstractNode::goDown() {
    std::cout << "Node " << id << " going DOWN" << std::endl;
    up.store(false);
}

void AbstractNode::goUp() {
    std::cout << "Node " << id << " coming UP" << std::endl;
    up.store(true);
}

std::string AbstractNode::read(const std::string& key) {
    if (!up.load()) {
        std::cout << "Node " << id << " is DOWN, cannot read" << std::endl;
        return "";
    }
    
    std::shared_lock<std::shared_mutex> readLock(lock);
    auto it = dataStore.find(key);
    if (it != dataStore.end()) {
        return it->second;
    }
    return "";
}

std::map<std::string, std::string> AbstractNode::getDataStore() const {
    if (!up.load()) {
        std::cout << "Node " << id << " is DOWN, cannot get data store" << std::endl;
        return {};
    }
    
    std::shared_lock<std::shared_mutex> readLock(lock);
    return dataStore;
}

int64_t AbstractNode::getLastLogIndex() const {
    if (!up.load()) {
        return -1;
    }
    return lastAppliedIndex.load();
}

bool AbstractNode::applyLogEntry(const LogEntry& entry, std::shared_mutex& entryLock) {
    if (!up.load()) {
        std::cout << "Node " << id << " is DOWN, cannot apply log entry" << std::endl;
        return false;
    }
    
    std::unique_lock<std::shared_mutex> writeLock(lock);
    
    // Check if this log entry is the next in sequence
    if (entry.getId() != lastAppliedIndex.load() + 1) {
        std::cout << "Node " << id << " received out-of-order log entry: " 
                 << entry.getId() << ", expected: " << (lastAppliedIndex.load() + 1) << std::endl;
        return false;
    }
    
    // Apply the log entry to the data store
    dataStore[entry.getKey()] = entry.getValue();
    
    // Add to log and update index
    log.push_back(entry);
    lastAppliedIndex.store(entry.getId());
    
    std::cout << "Node " << id << " applied log entry: " << entry.toString() << std::endl;
    return true;
}

std::vector<LogEntry> AbstractNode::getLogEntriesAfter(int64_t afterIndex) const {
    if (!up.load()) {
        std::cout << "Node " << id << " is DOWN, cannot get log entries" << std::endl;
        return {};
    }
    
    std::shared_lock<std::shared_mutex> readLock(lock);
    std::vector<LogEntry> entries;
    for (const LogEntry& entry : log) {
        if (entry.getId() > afterIndex) {
            entries.push_back(entry);
        }
    }
    return entries;
}