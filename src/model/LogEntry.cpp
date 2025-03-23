#include "model/LogEntry.h"
#include <sstream>

LogEntry::LogEntry(int64_t id, const std::string& key, const std::string& value)
    : id(id), key(key), value(value) {
    // Get current time in milliseconds since epoch
    auto now = std::chrono::system_clock::now();
    timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
}

int64_t LogEntry::getId() const {
    return id;
}

const std::string& LogEntry::getKey() const {
    return key;
}

const std::string& LogEntry::getValue() const {
    return value;
}

int64_t LogEntry::getTimestamp() const {
    return timestamp;
}

std::string LogEntry::toString() const {
    std::ostringstream oss;
    oss << "LogEntry{id=" << id
        << ", key='" << key << "'"
        << ", value='" << value << "'"
        << ", timestamp=" << timestamp << "}";
    return oss.str();
}