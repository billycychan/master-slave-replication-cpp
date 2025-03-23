// src/node/MasterNode.cpp
#include "node/MasterNode.h"
#include "node/SlaveNode.h"
#include <iostream>
#include <future>

MasterNode::MasterNode(const std::string& id)
    : AbstractNode(id), nextLogId(1) {
}

void MasterNode::registerSlave(std::shared_ptr<SlaveNode> slave) {
    slaves.insert(slave);
    std::cout << "Master " << id << " registered slave: " << slave->getId() << std::endl;
}

bool MasterNode::write(const std::string& key, const std::string& value) {
    if (!up.load()) {
        std::cout << "Master " << id << " is DOWN, cannot write" << std::endl;
        return false;
    }

    std::unique_lock<std::shared_mutex> writeLock(lock);
    
    // Create a new log entry
    LogEntry entry(nextLogId.fetch_add(1), key, value);
    
    // Apply to the master's data store first
    dataStore[key] = value;
    log.push_back(entry);
    lastAppliedIndex.store(entry.getId());
    
    std::cout << "Master " << id << " wrote " << key << "=" << value 
             << " (Log ID: " << entry.getId() << ")" << std::endl;
    
    // Track replication status
    pendingReplications[entry.getId()] = std::set<std::string>();
    
    // Asynchronously replicate to slaves
    replicateToSlaves(entry);
    
    return true;
}

void MasterNode::replicateToSlaves(const LogEntry& entry) {
    for (auto& slave : slaves) {
        auto task = std::async(std::launch::async, [this, slave, entry]() {
            if (slave->isUp()) {
                bool success = slave->applyLogEntry(entry, lock);
                if (success) {
                    // Track successful replication
                    auto slaveSet = pendingReplications.find(entry.getId());
                    if (slaveSet != pendingReplications.end()) {
                        slaveSet->second.insert(slave->getId());
                        std::cout << "Master " << id << " replicated log entry " << entry.getId() 
                                << " to slave " << slave->getId() << std::endl;
                    }
                } else {
                    recoverSlave(slave);
                }
            } else {
                std::cout << "Master " << id << " couldn't replicate to slave " 
                        << slave->getId() << " (DOWN)" << std::endl;
            }
        });
        replicationTasks.push_back(std::move(task));
    }
}

void MasterNode::recoverSlave(std::shared_ptr<SlaveNode> slave) {
    if (!up.load() || !slave->isUp()) {
        std::cout << "Master " << id << " or Slave " << slave->getId() 
                 << " is DOWN, cannot recover" << std::endl;
        return;
    }

    std::cout << "Master " << id << " starting recovery for slave " 
             << slave->getId() << std::endl;

    auto task = std::async(std::launch::async, [this, slave]() {
        int64_t slaveLastIndex = slave->getLastLogIndex();
        std::vector<LogEntry> missingEntries = getLogEntriesAfter(slaveLastIndex);

        std::cout << "Master " << id << " sending " << missingEntries.size() 
                << " log entries to slave " << slave->getId() << std::endl;

        for (const LogEntry& entry : missingEntries) {
            slave->applyLogEntry(entry, lock);
        }

        std::cout << "Master " << id << " completed recovery for slave " 
                << slave->getId() << " up to log index " << slave->getLastLogIndex() << std::endl;
    });
    
    replicationTasks.push_back(std::move(task));
}

void MasterNode::shutdown() {
    // Wait for all pending replication tasks to complete
    for (auto& task : replicationTasks) {
        if (task.valid()) {
            task.wait();
        }
    }
    replicationTasks.clear();
}