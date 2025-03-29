#ifndef MASTER_NODE_H
#define MASTER_NODE_H

#include "node/AbstractNode.h"
#include <set>
#include <unordered_map>
#include <atomic>
#include <memory>
#include <future>

namespace replication {
namespace node {

// Forward declaration
class SlaveNode;

/**
 * Implementation of the master node in the replication system.
 * The master node is responsible for handling write operations and replicating
 * them to slave nodes.
 */
class MasterNode : public AbstractNode {
public:
    /**
     * Constructs a master node with the given ID.
     */
    explicit MasterNode(const std::string& id);
    
    /**
     * Destructor
     */
    ~MasterNode() override;

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
     * Deletes a key-value pair from the master and replicates the delete operation to the slaves.
     * @param key the key to delete
     * @return true if the delete was successful
     */
    bool deleteKey(const std::string& key) override;
    
    /**
     * Shuts down the replication executor service.
     */
    void shutdown();

private:
    /**
     * Replicates a log entry to all registered slave nodes asynchronously.
     * @param entry the log entry to replicate
     */
    void replicateToSlaves(const model::LogEntry& entry);

    std::set<std::shared_ptr<SlaveNode>> slaves_;
    std::unordered_map<long, std::set<std::string>> pendingReplications_;
    std::atomic<long> nextLogId_;
    mutable std::mutex slavesMutex_;
    mutable std::mutex pendingReplicationsMutex_;
};

} // namespace node
} // namespace replication

#endif // MASTER_NODE_H