// src/node/MasterNode.h
#ifndef MASTER_NODE_H
#define MASTER_NODE_H

#include "AbstractNode.h"
#include <set>
#include <unordered_map>
#include <memory>

// Forward declaration to handle circular dependency
class SlaveNode;

/**
 * Implementation of the master node in the replication system.
 * The master node is responsible for handling write operations and replicating
 * them to slave nodes.
 */
class MasterNode : public AbstractNode {
private:
    std::set<std::shared_ptr<SlaveNode>> slaves;
    std::unordered_map<int64_t, std::set<std::string>> pendingReplications;
    std::atomic<int64_t> nextLogId;

    /**
     * Replicates a log entry to all registered slave nodes asynchronously.
     * @param entry the log entry to replicate
     */
    void replicateToSlaves(const LogEntry& entry);

public:
    /**
     * Constructor for the master node.
     * @param id the unique ID of the node
     */
    MasterNode(const std::string& id);
    
    /**
     * Registers a slave node with this master.
     * @param slave the slave node to register
     */
    void registerSlave(std::shared_ptr<SlaveNode> slave);
    
    /**
     * Writes a key-value pair to the master and replicates it to the slaves.
     * @param key the key to write
     * @param value the value to write
     * @return true if the write was successful
     */
    bool write(const std::string& key, const std::string& value);
    
    /**
     * Shuts down the master node, cleaning up any pending tasks.
     */
    void shutdown();
    
    /**
     * Makes a slave node recover by sending it all missing log entries.
     * @param slave the slave node to recover
     */
    void recoverSlave(std::shared_ptr<SlaveNode> slave);
};

#endif // MASTER_NODE_H