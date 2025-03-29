// tests/NodeTest.cpp
#include <gtest/gtest.h>
#include "node/MasterNode.h"
#include "node/SlaveNode.h"
#include <thread>
#include <chrono>

using namespace replication;
using namespace std::chrono_literals;

class NodeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a new system with master and slaves for each test
        master = std::make_shared<node::MasterNode>("test-master");
        slave1 = std::make_shared<node::SlaveNode>("test-slave-1", master);
        slave2 = std::make_shared<node::SlaveNode>("test-slave-2", master);
        
        // Register slaves with master
        master->registerSlave(slave1);
        master->registerSlave(slave2);
    }

    void TearDown() override {
        // Clean up after each test
        if (master) {
            master->shutdown();
        }
    }

    std::shared_ptr<node::MasterNode> master;
    std::shared_ptr<node::SlaveNode> slave1;
    std::shared_ptr<node::SlaveNode> slave2;
};

TEST_F(NodeTest, TestMasterNodeBasicOperations) {
    // Test master node identification
    EXPECT_EQ("test-master", master->getId());
    EXPECT_TRUE(master->isUp());

    // Test basic write operation
    EXPECT_TRUE(master->write("master-key", "master-value"));

    // Test master node can read its own data
    EXPECT_EQ("master-value", master->read("master-key"));
}

TEST_F(NodeTest, TestSlaveNodeBasicOperations) {
    // Test slave node identification
    EXPECT_EQ("test-slave-1", slave1->getId());
    EXPECT_TRUE(slave1->isUp());

    // Write to master and check replication to slave
    EXPECT_TRUE(master->write("key-for-slave", "value-for-slave"));
    std::this_thread::sleep_for(1s); // Wait for replication

    // Slave should have received the data
    EXPECT_EQ("value-for-slave", slave1->read("key-for-slave"));
}

TEST_F(NodeTest, TestSlaveNodeFailureAndRecovery) {
    // Initial write to master
    EXPECT_TRUE(master->write("key1", "value1"));
    std::this_thread::sleep_for(1s);

    // Verify slave has the data
    EXPECT_EQ("value1", slave1->read("key1"));

    // Simulate slave failure
    slave1->goDown();
    EXPECT_FALSE(slave1->isUp());
    EXPECT_EQ("", slave1->read("key1")); // Should return empty string when down

    // Write more data while slave is down
    EXPECT_TRUE(master->write("key2", "value2"));
    EXPECT_TRUE(master->write("key3", "value3"));

    // Bring slave back up
    slave1->goUp(); // This should trigger recovery
    std::this_thread::sleep_for(2s); // Wait for recovery

    // Verify slave has caught up with all data
    EXPECT_TRUE(slave1->isUp());
    EXPECT_EQ("value1", slave1->read("key1"));
    EXPECT_EQ("value2", slave1->read("key2"));
    EXPECT_EQ("value3", slave1->read("key3"));
}

TEST_F(NodeTest, TestMasterNodeFailure) {
    // Write data while master is up
    EXPECT_TRUE(master->write("pre-failure", "value"));

    // Simulate master failure
    master->goDown();
    EXPECT_FALSE(master->isUp());

    // Write should fail when master is down
    EXPECT_FALSE(master->write("post-failure", "value"));

    // Bring master back up
    master->goUp();
    EXPECT_TRUE(master->isUp());

    // Write should succeed again
    EXPECT_TRUE(master->write("post-recovery", "value"));
}

TEST_F(NodeTest, TestLogEntryReplication) {
    // Write several entries to create log entries
    for (int i = 0; i < 5; i++) {
        master->write("log-key-" + std::to_string(i), "log-value-" + std::to_string(i));
    }

    std::this_thread::sleep_for(1s);

    // Get log entries from master after index 0
    auto masterLogEntries = master->getLogEntriesAfter(0);
    EXPECT_EQ(5, masterLogEntries.size());

    // Get log entries from slave after index 0
    auto slaveLogEntries = slave1->getLogEntriesAfter(0);
    EXPECT_EQ(5, slaveLogEntries.size());

    // Compare log entries between master and slave
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(masterLogEntries[i].getId(), slaveLogEntries[i].getId());
        EXPECT_EQ(masterLogEntries[i].getKey(), slaveLogEntries[i].getKey());
        EXPECT_EQ(masterLogEntries[i].getValue(), slaveLogEntries[i].getValue());
    }
}