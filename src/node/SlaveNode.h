// SlaveNode.h - revised
#ifndef SLAVE_NODE_H
#define SLAVE_NODE_H

#include "node/AbstractNode.h"
#include <memory>

namespace replication {
namespace node {

// Forward declaration
class MasterNode;

/**
 * Implementation of a slave node in the replication system.
 * Slave nodes receive and apply log entries from the master,
 * and handle read operations.
 */
class SlaveNode : public AbstractNode, 
                 public std::enable_shared_from_this<SlaveNode> {
public:
    /**
     * Constructs a slave node with the given ID and master reference.
     * @param id The slave node's ID
     * @param master The master node reference
     */
    SlaveNode(const std::string& id, std::shared_ptr<MasterNode> master);
    
    /**
     * Destructor
     */
    ~SlaveNode() override = default;

    /**
     * Requests recovery from the master node.
     * This is called when a slave node comes back up after being down.
     */
    void requestRecovery();
    
    /**
     * Recovers a slave node by sending it all missing log entries.
     */
    void recoverSlave();
    
    /**
     * Brings the node back up after a failure.
     * Overrides the base implementation to also trigger recovery.
     */
    void goUp() override;

private:
    std::shared_ptr<MasterNode> master_;
};

} // namespace node
} // namespace replication

#endif // SLAVE_NODE_H