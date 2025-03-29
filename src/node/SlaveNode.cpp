// SlaveNode.cpp
#include "node/SlaveNode.h"
#include "node/MasterNode.h"
#include <iostream>

namespace replication {
namespace node {

SlaveNode::SlaveNode(const std::string& id, std::shared_ptr<MasterNode> master)
    : AbstractNode(id),
      master_(master) {
    // We need to defer registration until the object is fully constructed
}

void SlaveNode::requestRecovery() {
    if (!up_) {
        std::cout << "Slave " << id_ << " is DOWN, cannot request recovery" << std::endl;
        return;
    }

    std::cout << "Slave " << id_ << " requesting recovery from master" << std::endl;
    recoverSlave();
}

void SlaveNode::goUp() {
    AbstractNode::goUp();
    // When coming back up, request recovery from the master
    requestRecovery();
}

void SlaveNode::recoverSlave() {
    if (!up_ || !master_->isUp()) {
        std::cout << "Master or Slave " << id_ << " is DOWN, cannot recover" << std::endl;
        return;
    }

    std::cout << "Master starting recovery for slave " << id_ << std::endl;

    replicationExecutor_->enqueue([this]() {
        long slaveLastIndex = this->getLastLogIndex();
        std::vector<model::LogEntry> missingEntries = master_->getLogEntriesAfter(slaveLastIndex);

        std::cout << "Master sending " << missingEntries.size() 
                  " log entries to slave " << this->id_ << std::endl;

        for (const auto& entry : missingEntries) {
            this->applyLogEntry(entry, lock_);
        }

        std::cout << "Master completed recovery for slave " 
                  this->id_ << " up to log index " << this->lastAppliedIndex_ << std::endl;
    });
}

} // namespace node
} // namespace replication