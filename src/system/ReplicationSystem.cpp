#include "system/ReplicationSystem.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <random>

namespace replication {
namespace system {

ReplicationSystem::ReplicationSystem(int numSlaves)
    : random_(std::random_device()()),  // Seed the random generator
      stopFailureSimulator_(true),
      failureProbability_(0.0),
      recoveryProbability_(0.0),
      checkIntervalSeconds_(0) {
    
    // Create master node
    master_ = std::make_shared<node::MasterNode>("master");
    
    // Create slave nodes
    for (int i = 0; i < numSlaves; i++) {
        std::string slaveId = "slave-" + std::to_string(i);
        auto slave = std::make_shared<node::SlaveNode>(slaveId, master_);
        slaves_.push_back(slave);
        // Register slave with master
        master_->registerSlave(slave);
    }
    
    std::cout << "Replication system initialized with 1 master and " 
              << numSlaves << " slaves" << std::endl;
}

ReplicationSystem::~ReplicationSystem() {
    shutdown();
}

bool ReplicationSystem::write(const std::string& key, const std::string& value) {
    return master_->write(key, value);
}

bool ReplicationSystem::deleteKey(const std::string& key) {
    return master_->deleteKey(key);
}

std::string ReplicationSystem::read(const std::string& key) {
    // Try to get a working slave
    std::shared_ptr<node::SlaveNode> slave = getRandomUpSlave();
    if (!slave) {
        std::cout << "All slaves are DOWN, cannot read" << std::endl;
        return "";
    }
    
    std::string value = slave->read(key);
    std::cout << "Read " << key << "=" << value << " from " << slave->getId() << std::endl;
    return value;
}

std::shared_ptr<node::SlaveNode> ReplicationSystem::getRandomUpSlave() const {
    std::vector<std::shared_ptr<node::SlaveNode>> upSlaves;
    
    for (const auto& slave : slaves_) {
        if (slave->isUp()) {
            upSlaves.push_back(slave);
        }
    }
    
    if (upSlaves.empty()) {
        return nullptr;
    }
    
    // Get a random up slave
    std::lock_guard<std::mutex> lock(randomMutex_);
    std::uniform_int_distribution<int> dist(0, upSlaves.size() - 1);
    return upSlaves[dist(random_)];
}

std::map<std::string, std::string> ReplicationSystem::getDataStore() const {
    std::shared_ptr<node::SlaveNode> slave = getRandomUpSlave();
    if (!slave) {
        std::cout << "All slaves are DOWN, cannot get data store" << std::endl;
        return {};
    }
    
    return slave->getDataStore();
}

void ReplicationSystem::startFailureSimulator(double failureProbability, 
                                             double recoveryProbability, 
                                             int checkIntervalSeconds) {
    // Stop any existing simulator thread
    if (!stopFailureSimulator_) {
        shutdown();
    }
    
    failureProbability_ = failureProbability;
    recoveryProbability_ = recoveryProbability;
    checkIntervalSeconds_ = checkIntervalSeconds;
    stopFailureSimulator_ = false;
    
    // Start the simulator thread
    failureSimulatorThread_ = std::thread(&ReplicationSystem::failureSimulatorThread, this);
    
    std::cout << "Started failure simulator with check interval " 
              << checkIntervalSeconds << " seconds" << std::endl;
}

void ReplicationSystem::failureSimulatorThread() {
    while (!stopFailureSimulator_) {
        {
            std::unique_lock<std::mutex> lock(failureSimulatorMutex_);
            failureSimulatorCV_.wait_for(lock, 
                                      std::chrono::seconds(checkIntervalSeconds_),
                                      [this] { return stopFailureSimulator_.load(); });
            
            if (stopFailureSimulator_) {
                break;
            }
        }
        
        // Run the failure and recovery simulation
        simulateFailureAndRecovery(failureProbability_, recoveryProbability_);
    }
}

void ReplicationSystem::simulateFailureAndRecovery(double failureProbability, 
                                                 double recoveryProbability) {
    std::lock_guard<std::mutex> lock(randomMutex_);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    for (auto& slave : slaves_) {
        if (slave->isUp() && dist(random_) < failureProbability) {
            slave->goDown();
        } else if (!slave->isUp() && dist(random_) < recoveryProbability) {
            slave->goUp();
        }
    }
}

std::vector<model::LogEntry> ReplicationSystem::getLogs() const {
    if (!master_->isUp()) {
        std::cout << "Master is DOWN, cannot get logs" << std::endl;
        return {};
    }
    
    return master_->getLogEntriesAfter(0); // Get all logs from the beginning
}

std::map<std::string, bool> ReplicationSystem::getNodesStatus() const {
    std::map<std::string, bool> status;
    
    // Add master status
    status[master_->getId()] = master_->isUp();
    
    // Add slave statuses
    for (const auto& slave : slaves_) {
        status[slave->getId()] = slave->isUp();
    }
    
    return status;
}

void ReplicationSystem::shutdown() {
    // Signal the failure simulator to stop
    {
        std::lock_guard<std::mutex> lock(failureSimulatorMutex_);
        stopFailureSimulator_ = true;
    }
    failureSimulatorCV_.notify_all();
    
    // Join the failure simulator thread if it's running
    if (failureSimulatorThread_.joinable()) {
        failureSimulatorThread_.join();
    }
    
    // Shutdown the master node
    master_->shutdown();
    
    std::cout << "Replication system shut down" << std::endl;
}

} // namespace system
} // namespace replication