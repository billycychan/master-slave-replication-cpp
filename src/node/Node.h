// src/node/Node.h
#ifndef NODE_H
#define NODE_H

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include "model/LogEntry.h"

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
     * Gets a copy of the entire data store.
     * @return a copy of the data store
     */
    virtual std::map<std::string, std::string> getDataStore() const = 0;
    
    /**
     * Gets the last log index that this node has processed.
     * @return the last log index
     */
    virtual int64_t getLastLogIndex() const = 0;
    
    /**
     * Applies a log entry to this node.
     * @param entry the log entry to apply
     * @param lock the lock to use for synchronization
     * @return true if applied successfully
     */
    virtual bool applyLogEntry(const LogEntry& entry, std::shared_mutex& lock) = 0;
    
    /**
     * Gets all log entries after the specified index.
     * @param afterIndex the index after which to get log entries
     * @return a vector of log entries
     */
    virtual std::vector<LogEntry> getLogEntriesAfter(int64_t afterIndex) const = 0;
};

#endif // NODE_H