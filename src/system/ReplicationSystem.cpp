// src/system/ReplicationSystem.cpp
#include "system/ReplicationSystem.h"
#include <iostream>
#include <algorithm>
#include <chrono>

ReplicationSystem::ReplicationSystem(int numSlaves)
    : rng(std::random_device()()),  // Seed the random number generator
      distribution(0.0, 1.0),       // Uniform distribution between 0 and 1
      running(false) {
    
    // Create master node
    master = std::make_shared<MasterNode>("master");
    
    // Create slave nodes
    for (int i = 0; i < numSlaves; i++) {
        auto slave = std::make_shared<SlaveNode>("slave-" + std::to_string(i), master);
        slaves.push_back(slave);
        master->registerSlave(slave);
    }
    
    std::cout << "Replication system initialized with 1 master and " << numSlaves << " slaves" << std::endl;
}

ReplicationSystem::~ReplicationSystem() {
    shutdown();
}

bool ReplicationSystem::write(const std::string& key, const std::string& value) {
    return master->write(key, value);
}

std::string ReplicationSystem::read(const std::string& key) {
    // Try to get a working slave
    auto slave = getRandomUpSlave();
    if (!slave) {
        std::cout << "All slaves are DOWN, cannot read" << std::endl;
        return "";
    }
    
    std::string value = slave->read(key);
    std::cout << "Read " << key << "=" << value << " from " << slave->getId() << std::endl;
    return value;
}

std::shared_ptr<SlaveNode> ReplicationSystem::getRandomUpSlave() const {
    std::vector<std::shared_ptr<SlaveNode>> upSlaves;
    for (const auto& slave : slaves) {
        if (slave->isUp()) {
            upSlaves.push_back(slave);
        }
    }
    
    if (upSlaves.empty()) {
        return nullptr;
    }
    
    // Generate a random index in the range [0, upSlaves.size() - 1]
    std::uniform_int_distribution<size_t> indexDist(0, upSlaves.size() - 1);
    size_t randomIndex = indexDist(rng);
    return upSlaves[randomIndex];
}

std::map<std::string, std::string> ReplicationSystem::getDataStore() const {
    // Find a random slave that is up
    std::vector<std::shared_ptr<SlaveNode>> upSlaves;
    for (const auto& slave : slaves) {
        if (slave->isUp()) {
            upSlaves.push_back(slave);
        }
    }
    
    if (upSlaves.empty()) {
        std::cout << "All slaves are DOWN, cannot get data store" << std::endl;
        return {};
    }
    
    // Select a random up slave
    std::uniform_int_distribution<size_t> indexDist(0, upSlaves.size() - 1);
    size_t randomIndex = indexDist(rng);
    return upSlaves[randomIndex]->getDataStore();
}

void ReplicationSystem::startFailureSimulator(double failureProbability, double recoveryProbability, int checkIntervalSeconds) {
    if (running.load()) {
        std::cout << "Failure simulator is already running" << std::endl;
        return;
    }
    
    running.store(true);
    simulatorThread = std::thread(&ReplicationSystem::failureSimulatorThread, 
                                  this, failureProbability, recoveryProbability, checkIntervalSeconds);
    
    std::cout << "Started failure simulator with check interval " 
            << checkIntervalSeconds << " seconds" << std::endl;
}

void ReplicationSystem::failureSimulatorThread(double failureProbability, double recoveryProbability, int checkIntervalSeconds) {
    while (running.load()) {
        simulateFailureAndRecovery(failureProbability, recoveryProbability);
        
        // Sleep for the check interval
        std::unique_lock<std::mutex> lock(simulatorMutex);
        simulatorCV.wait_for(lock, std::chrono::seconds(checkIntervalSeconds), 
                            [this]() { return !running.load(); });
    }
}

void ReplicationSystem::simulateFailureAndRecovery(double failureProbability, double recoveryProbability) {
    for (auto& slave : slaves) {
        if (slave->isUp() && distribution(rng) < failureProbability) {
            slave->goDown();
        } else if (!slave->isUp() && distribution(rng) < recoveryProbability) {
            slave->goUp();
        }
    }
}

void ReplicationSystem::shutdown() {
    // Stop the failure simulator thread
    if (running.load()) {
        running.store(false);
        simulatorCV.notify_all();
        if (simulatorThread.joinable()) {
            simulatorThread.join();
        }
    }
    
    // Shutdown the master node
    if (master) {
        master->shutdown();
    }
    
    std::cout << "Replication system shut down" << std::endl;
}