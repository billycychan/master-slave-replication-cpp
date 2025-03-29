#include "model/LogEntry.h"
#include <chrono>
#include <sstream>

namespace replication {
namespace model {

LogEntry::LogEntry(long id, const std::string& key, const std::string& value)
    : id_(id), key_(key), value_(value), operationType_(OperationType::WRITE) {
    // Get current time in milliseconds since epoch
    auto now = std::chrono::system_clock::now();
    timestamp_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
}

LogEntry::LogEntry(long id, const std::string& key, const std::string& value, 
                   OperationType operationType)
    : id_(id), key_(key), value_(value), operationType_(operationType) {
    // Get current time in milliseconds since epoch
    auto now = std::chrono::system_clock::now();
    timestamp_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
}

long LogEntry::getId() const {
    return id_;
}

const std::string& LogEntry::getKey() const {
    return key_;
}

const std::string& LogEntry::getValue() const {
    return value_;
}

long LogEntry::getTimestamp() const {
    return timestamp_;
}

LogEntry::OperationType LogEntry::getOperationType() const {
    return operationType_;
}

bool LogEntry::isDelete() const {
    return operationType_ == OperationType::DELETE;
}

std::string LogEntry::toString() const {
    std::ostringstream oss;
    oss << "LogEntry{id=" << id_
        << ", key='" << key_ << "'"
        << ", value='" << value_ << "'"
        << ", timestamp=" << timestamp_
        << ", operation=" << (isDelete() ? "DELETE" : "WRITE") << "}";
    return oss.str();
}

} // namespace model
} // namespace replication