// src/node/SlaveNode.h
#ifndef SLAVE_NODE_H
#define SLAVE_NODE_H

#include "AbstractNode.h"
#include <memory>

// Forward declaration to handle circular dependency
class MasterNode;

/**
 * Implementation of a slave node in the replication system.
 * Slave nodes receive and apply log entries from the master,
 * and handle read operations.
 */
class SlaveNode : public AbstractNode, public std::enable_shared_from_this<SlaveNode> {
private:
    std::shared_ptr<MasterNode> master;

public:
    /**
     * Constructor for a slave node.
     * @param id the unique ID of the node
     * @param master the master node this slave belongs to
     */
    SlaveNode(const std::string& id, std::shared_ptr<MasterNode> master);
    
    /**
     * Requests recovery from the master node.
     * This is called when a slave node comes back up after being down.
     */
    void requestRecovery();
    
    /**
     * Overrides goUp to request recovery when coming back up.
     */
    void goUp() override;
    
    /**
     * Triggers the recovery process for this slave.
     */
    void recoverSlave();
};

#endif // SLAVE_NODE_H