#ifndef LOG_ENTRY_H
#define LOG_ENTRY_H

#include <string>
#include <ctime>

namespace replication {
namespace model {

/**
 * Represents a log entry in the replication log.
 * Each entry contains information about a write or delete operation.
 */
class LogEntry {
public:
    /**
     * Enum representing the type of operation in the log entry.
     */
    enum class OperationType {
        WRITE,
        DELETE
    };

    /**
     * Creates a new log entry for a write operation.
     * @param id the log entry ID
     * @param key the key being written
     * @param value the value being written
     */
    LogEntry(long id, const std::string& key, const std::string& value);

    /**
     * Creates a new log entry with a specified operation type.
     * @param id the log entry ID
     * @param key the key being operated on
     * @param value the value (for write operations, empty for delete operations)
     * @param operationType the type of operation
     */
    LogEntry(long id, const std::string& key, const std::string& value, 
             OperationType operationType);

    // Getters
    long getId() const;
    const std::string& getKey() const;
    const std::string& getValue() const;
    long getTimestamp() const;
    OperationType getOperationType() const;

    /**
     * Checks if this log entry is a delete operation.
     * @return true if this is a delete operation, false otherwise
     */
    bool isDelete() const;

    /**
     * Convert log entry to string representation
     */
    std::string toString() const;

private:
    long id_;
    std::string key_;
    std::string value_;
    long timestamp_;
    OperationType operationType_;
};

} // namespace model
} // namespace replication

#endif // LOG_ENTRY_H