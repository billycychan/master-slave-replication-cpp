#ifndef NODE_H
#define NODE_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <shared_mutex>  // For read-write lock

#include "model/LogEntry.h"

namespace replication {
namespace node {

/**
 * Interface representing a node in the replication system.
 * Both master and slave nodes implement this interface.
 */
class Node {
public:
    virtual ~Node() = default;

    /**
     * Gets the unique ID of this node.
     * @return the node ID
     */
    virtual std::string getId() const = 0;
    
    /**
     * Gets the current state of this node (UP or DOWN).
     * @return true if the node is up, false otherwise
     */
    virtual bool isUp() const = 0;
    
    /**
     * Brings the node down to simulate failure.
     */
    virtual void goDown() = 0;
    
    /**
     * Brings the node back up after a failure.
     */
    virtual void goUp() = 0;
    
    /**
     * Reads a value from the node's data store.
     * @param key the key to read
     * @return the value, or empty string if not found
     */
    virtual std::string read(const std::string& key) = 0;
    
    /**
     * Deletes a key-value pair from the node's data store.
     * @param key the key to delete
     * @return true if the key was found and deleted, false otherwise
     */
    virtual bool deleteKey(const std::string& key) = 0;
    
    /**
     * Gets a copy of the entire data store.
     * @return a copy of the data store
     */
    virtual std::map<std::string, std::string> getDataStore() const = 0;
    
    /**
     * Gets the last log index that this node has processed.
     * @return the last log index
     */
    virtual long getLastLogIndex() const = 0;
    
    /**
     * Applies a log entry to this node.
     * @param entry the log entry to apply
     * @param lock the lock to use for thread safety
     * @return true if applied successfully
     */
    virtual bool applyLogEntry(const model::LogEntry& entry, 
                              std::shared_ptr<std::shared_mutex> lock) = 0;
    
    /**
     * Gets all log entries after the specified index.
     * @param afterIndex the index after which to get log entries
     * @return a list of log entries
     */
    virtual std::vector<model::LogEntry> getLogEntriesAfter(long afterIndex) const = 0;
};

} // namespace node
} // namespace replication

#endif // NODE_H