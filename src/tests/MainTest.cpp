// tests/MainTest.cpp
#include <gtest/gtest.h>
#include "system/ReplicationSystem.h"
#include <thread>
#include <chrono>

using namespace replication;
using namespace std::chrono_literals;

class MainTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a new system with 3 slaves for each test
        system = std::make_unique<system::ReplicationSystem>(3);
    }

    void TearDown() override {
        // Clean up after each test
        if (system) {
            system->shutdown();
        }
    }

    std::unique_ptr<system::ReplicationSystem> system;
};

TEST_F(MainTest, TestBasicWriteAndRead) {
    // Basic write/read functionality
    EXPECT_TRUE(system->write("key1", "value1"));
    std::this_thread::sleep_for(1s); // Wait for replication
    EXPECT_EQ("value1", system->read("key1"));
}

TEST_F(MainTest, TestMultipleWritesAndReads) {
    // Write multiple entries
    EXPECT_TRUE(system->write("key1", "value1"));
    EXPECT_TRUE(system->write("key2", "value2"));
    EXPECT_TRUE(system->write("key3", "value3"));

    // Wait for replication
    std::this_thread::sleep_for(1s);

    // Read all entries
    EXPECT_EQ("value1", system->read("key1"));
    EXPECT_EQ("value2", system->read("key2"));
    EXPECT_EQ("value3", system->read("key3"));

    // Verify non-existent key returns empty string
    EXPECT_EQ("", system->read("nonexistent"));
}

TEST_F(MainTest, TestUpdateExistingKey) {
    // Write initial value
    EXPECT_TRUE(system->write("key1", "initial"));
    std::this_thread::sleep_for(1s);
    EXPECT_EQ("initial", system->read("key1"));

    // Update with new value
    EXPECT_TRUE(system->write("key1", "updated"));
    std::this_thread::sleep_for(1s);
    EXPECT_EQ("updated", system->read("key1"));
}

TEST_F(MainTest, TestDataStoreConsistency) {
    // Write several entries
    EXPECT_TRUE(system->write("key1", "value1"));
    EXPECT_TRUE(system->write("key2", "value2"));
    std::this_thread::sleep_for(1s);

    // Get the data store and check contents
    auto dataStore = system->getDataStore();
    EXPECT_EQ("value1", dataStore["key1"]);
    EXPECT_EQ("value2", dataStore["key2"]);
    EXPECT_EQ(2, dataStore.size());
}

TEST_F(MainTest, TestEmptyDataStore) {
    // New system should have empty data store
    auto dataStore = system->getDataStore();
    EXPECT_TRUE(dataStore.empty());
}