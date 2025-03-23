// src/node/SlaveNode.cpp
#include "node/SlaveNode.h"
#include "node/MasterNode.h"
#include <iostream>

SlaveNode::SlaveNode(const std::string& id, std::shared_ptr<MasterNode> master)
    : AbstractNode(id), master(master) {
    // Register with the master (this is handled in the caller in our C++ implementation)
}

void SlaveNode::requestRecovery() {
    if (!up.load()) {
        std::cout << "Slave " << id << " is DOWN, cannot request recovery" << std::endl;
        return;
    }

    std::cout << "Slave " << id << " requesting recovery from master" << std::endl;
    recoverSlave();
}

void SlaveNode::goUp() {
    AbstractNode::goUp();
    // When coming back up, request recovery from the master
    requestRecovery();
}

void SlaveNode::recoverSlave() {
    if (!up.load() || !master || !master->isUp()) {
        std::cout << "Slave " << id << " or Master is DOWN, cannot recover" << std::endl;
        return;
    }
    
    master->recoverSlave(std::static_pointer_cast<SlaveNode>(shared_from_this()));
}