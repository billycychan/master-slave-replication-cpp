// src/node/AbstractNode.h
#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

#include "Node.h"
#include <string>
#include <map>
#include <vector>
#include <shared_mutex>
#include <thread>
#include <atomic>
#include <future>
#include <iostream>

/**
 * Abstract base class for nodes in the replication system.
 * Provides common functionality for both master and slave nodes.
 */
class AbstractNode : public Node {
protected:
    std::string id;
    std::atomic<bool> up;
    std::map<std::string, std::string> dataStore;
    std::vector<LogEntry> log;
    mutable std::shared_mutex lock;
    std::atomic<int64_t> lastAppliedIndex;
    
    // Thread pool for replication tasks
    std::vector<std::future<void>> replicationTasks;

public:
    /**
     * Constructor for a node.
     * @param id the unique ID of the node
     */
    AbstractNode(const std::string& id);
    
    /**
     * Destructor to clean up resources.
     */
    virtual ~AbstractNode();
    
    // Implement Node interface methods
    std::string getId() const override;
    bool isUp() const override;
    void goDown() override;
    void goUp() override;
    std::string read(const std::string& key) override;
    std::map<std::string, std::string> getDataStore() const override;
    int64_t getLastLogIndex() const override;
    bool applyLogEntry(const LogEntry& entry, std::shared_mutex& lock) override;
    std::vector<LogEntry> getLogEntriesAfter(int64_t afterIndex) const override;
};

#endif // ABSTRACT_NODE_H