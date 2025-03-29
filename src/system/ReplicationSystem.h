#ifndef REPLICATION_SYSTEM_H
#define REPLICATION_SYSTEM_H

#include "node/MasterNode.h"
#include "node/SlaveNode.h"
#include "model/LogEntry.h"

#include <string>
#include <vector>
#include <memory>
#include <random>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace replication {
namespace system {

/**
 * Manager class for the entire replication system.
 * It manages master and slave nodes, and provides a simple API
 * for interacting with the replication system.
 */
class ReplicationSystem {
public:
    /**
     * Creates a new replication system with a master and the specified number of slaves.
     * @param numSlaves the number of slave nodes to create
     */
    explicit ReplicationSystem(int numSlaves);
    
    /**
     * Destructor that ensures proper cleanup
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
     * Deletes a key-value pair from the master.
     * @param key the key to delete
     * @return true if the delete was successful
     */
    bool deleteKey(const std::string& key);

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
    void startFailureSimulator(double failureProbability, 
                              double recoveryProbability, 
                              int checkIntervalSeconds);

    /**
     * Gets all log entries from the master node.
     * @return a list of all log entries from the master
     */
    std::vector<model::LogEntry> getLogs() const;
    
    /**
     * Gets the status of all nodes in the system.
     * @return a map of node IDs to their status (UP/DOWN)
     */
    std::map<std::string, bool> getNodesStatus() const;
    
    /**
     * Shuts down the replication system.
     */
    void shutdown();

private:
    /**
     * Gets a random slave that is up (not failed).
     * @return a random up slave, or nullptr if all slaves are down
     */
    std::shared_ptr<node::SlaveNode> getRandomUpSlave() const;
    
    /**
     * Simulates node failures and recoveries.
     * @param failureProbability the probability of a node failing
     * @param recoveryProbability the probability of a failed node recovering
     */
    void simulateFailureAndRecovery(double failureProbability, double recoveryProbability);
    
    /**
     * The failure simulator thread function
     */
    void failureSimulatorThread();

    std::shared_ptr<node::MasterNode> master_;
    std::vector<std::shared_ptr<node::SlaveNode>> slaves_;
    mutable std::mt19937 random_;  // Mersenne Twister random number generator
    mutable std::mutex randomMutex_;
    
    // Failure simulator control
    std::thread failureSimulatorThread_;
    std::atomic<bool> stopFailureSimulator_;
    double failureProbability_;
    double recoveryProbability_;
    int checkIntervalSeconds_;
    std::mutex failureSimulatorMutex_;
    std::condition_variable failureSimulatorCV_;
};

} // namespace system
} // namespace replication

#endif // REPLICATION_SYSTEM_H