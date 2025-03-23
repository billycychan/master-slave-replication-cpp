#ifndef LOG_ENTRY_H
#define LOG_ENTRY_H

#include <string>
#include <chrono>

/**
 * Represents a log entry in the replication log.
 * Each entry contains information about a write operation.
 */
class LogEntry {
private:
    int64_t id;
    std::string key;
    std::string value;
    int64_t timestamp;

public:
    /**
     * Constructor for a log entry.
     * @param id The unique identifier for the log entry
     * @param key The key of the data being updated
     * @param value The value of the data being updated
     */
    LogEntry(int64_t id, const std::string& key, const std::string& value);

    /**
     * Gets the unique identifier of the log entry.
     * @return The log entry ID
     */
    int64_t getId() const;

    /**
     * Gets the key of the data being updated.
     * @return The key
     */
    const std::string& getKey() const;

    /**
     * Gets the value of the data being updated.
     * @return The value
     */
    const std::string& getValue() const;

    /**
     * Gets the timestamp when the log entry was created.
     * @return The timestamp in milliseconds since epoch
     */
    int64_t getTimestamp() const;

    /**
     * Returns a string representation of the log entry.
     * @return A string representation
     */
    std::string toString() const;
};

#endif // LOG_ENTRY_H