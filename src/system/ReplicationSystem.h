// src/system/ReplicationSystem.h
#ifndef REPLICATION_SYSTEM_H
#define REPLICATION_SYSTEM_H

#include "node/MasterNode.h"
#include "node/SlaveNode.h"
#include <random>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <memory>

/**
 * Manager class for the entire replication system.
 * It manages master and slave nodes, and provides a simple API
 * for interacting with the replication system.
 */
class ReplicationSystem {
private:
    std::shared_ptr<MasterNode> master;
    std::vector<std::shared_ptr<SlaveNode>> slaves;
    mutable std::mt19937 rng;  // Mark as mutable so it can be used in const methods
    std::uniform_real_distribution<double> distribution;
    
    // For failure simulator
    std::thread simulatorThread;
    std::atomic<bool> running;
    std::mutex simulatorMutex;
    std::condition_variable simulatorCV;
    
    /**
     * Gets a random slave that is up (not failed).
     * @return a random up slave, or nullptr if all slaves are down
     */
    std::shared_ptr<SlaveNode> getRandomUpSlave() const;
    
    /**
     * Simulates node failures and recoveries.
     * @param failureProbability the probability of a node failing
     * @param recoveryProbability the probability of a failed node recovering
     */
    void simulateFailureAndRecovery(double failureProbability, double recoveryProbability);
    
    /**
     * Thread function for the failure simulator.
     */
    void failureSimulatorThread(double failureProbability, double recoveryProbability, int checkIntervalSeconds);

public:
    /**
     * Creates a new replication system with a master and the specified number of slaves.
     * @param numSlaves the number of slave nodes to create
     */
    ReplicationSystem(int numSlaves);
    
    /**
     * Destructor to clean up resources.
     */
    ~ReplicationSystem();
    
    /**
     * Writes a key-value pair to the master.
     * @param key the key to write
     * @param value the value to write
     * @return true if the write was successful
     */
    bool write(const std::string& key, const std::string& value);
    
    /**
     * Reads a value from a random slave node.
     * If the chosen slave is down, tries another slave.
     * @param key the key to read
     * @return the value, or empty string if not found or all slaves are down
     */
    std::string read(const std::string& key);
    
    /**
     * Gets the data store of a random slave that is up.
     * @return the data store, or empty map if all slaves are down
     */
    std::map<std::string, std::string> getDataStore() const;
    
    /**
     * Starts the failure simulator, which will randomly bring nodes down and up.
     * @param failureProbability the probability of a node failing in each check
     * @param recoveryProbability the probability of a failed node recovering in each check
     * @param checkIntervalSeconds the interval between checks in seconds
     */
    void startFailureSimulator(double failureProbability, double recoveryProbability, int checkIntervalSeconds);
    
    /**
     * Shuts down the replication system.
     */
    void shutdown();
};

#endif // REPLICATION_SYSTEM_H