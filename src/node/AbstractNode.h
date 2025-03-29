#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

#include "node/Node.h"
#include "model/LogEntry.h"

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <queue>
#include <functional>

namespace replication {
namespace node {

/**
 * Abstract base class for nodes in the replication system.
 * Provides common functionality for both master and slave nodes.
 */
class AbstractNode : public Node {
public:
    /**
     * Constructs a node with the given ID.
     */
    explicit AbstractNode(const std::string& id);
    
    /**
     * Virtual destructor to ensure proper cleanup.
     */
    virtual ~AbstractNode();

    // Node interface implementation
    std::string getId() const override;
    bool isUp() const override;
    void goDown() override;
    void goUp() override;
    std::string read(const std::string& key) override;
    bool deleteKey(const std::string& key) override;
    std::map<std::string, std::string> getDataStore() const override;
    long getLastLogIndex() const override;
    bool applyLogEntry(const model::LogEntry& entry, 
                      std::shared_ptr<std::shared_mutex> lock) override;
    std::vector<model::LogEntry> getLogEntriesAfter(long afterIndex) const override;

protected:
    // Thread pool implementation for asynchronous execution
    class ThreadPool {
    public:
        explicit ThreadPool(size_t num_threads);
        ~ThreadPool();
        
        template<class F>
        void enqueue(F&& f);
        
    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        
        std::mutex queue_mutex;
        std::condition_variable condition;
        bool stop;
    };
    
    std::string id_;
    std::atomic<bool> up_;
    std::map<std::string, std::string> dataStore_;
    std::vector<model::LogEntry> log_;
    std::shared_ptr<std::shared_mutex> lock_;
    std::atomic<long> lastAppliedIndex_;
    std::unique_ptr<ThreadPool> replicationExecutor_;
    
    // Mutex for thread-safe access to the log and data store
    mutable std::mutex logMutex_;
    mutable std::mutex dataStoreMutex_;
};

// Template implementation must be in the header
template<class F>
void AbstractNode::ThreadPool::enqueue(F&& f) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if(stop) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        tasks.emplace(std::forward<F>(f));
    }
    condition.notify_one();
}

} // namespace node
} // namespace replication

#endif // ABSTRACT_NODE_H