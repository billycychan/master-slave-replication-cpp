// tests/FaultToleranceTest.cpp
#include <gtest/gtest.h>
#include "system/ReplicationSystem.h"
#include <thread>
#include <chrono>
#include <unordered_map>
#include <random>

using namespace replication;
using namespace std::chrono_literals;

class FaultToleranceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a system with 5 slaves for more realistic fault tolerance testing
        system = std::make_unique<system::ReplicationSystem>(5);
    }

    void TearDown() override {
        // Clean up after each test
        if (system) {
            system->shutdown();
        }
    }

    std::unique_ptr<system::ReplicationSystem> system;
};

TEST_F(FaultToleranceTest, TestSlaveRecovery) {
    // Write initial data
    for (int i = 0; i < 5; i++) {
        EXPECT_TRUE(system->write("recovery-key-" + std::to_string(i), 
                                 "recovery-value-" + std::to_string(i)));
    }

    // Wait for replication
    std::this_thread::sleep_for(2s);

    // Simulate all slaves failing then recovering
    // First force high failure rate
    system->startFailureSimulator(1.0, 0.0, 1);
    std::this_thread::sleep_for(3s); // Time for all slaves to fail

    // Now force recovery
    system->startFailureSimulator(0.0, 1.0, 1);
    std::this_thread::sleep_for(5s); // Time for recovery

    // After recovery, read should succeed for all initial data
    for (int i = 0; i < 5; i++) {
        std::string key = "recovery-key-" + std::to_string(i);
        std::string expected = "recovery-value-" + std::to_string(i);
        EXPECT_EQ(expected, system->read(key));
    }
}

TEST_F(FaultToleranceTest, TestConsecutiveReadsWithFailures) {
    // Setup data
    for (int i = 0; i < 5; i++) {
        EXPECT_TRUE(system->write("read-key-" + std::to_string(i), 
                                "read-value-" + std::to_string(i)));
    }
    std::this_thread::sleep_for(1s);

    // Start failure simulator with moderate failure/recovery
    system->startFailureSimulator(0.4, 0.4, 1);

    // Perform multiple consecutive reads
    int successfulReads = 0;
    for (int i = 0; i < 20; i++) {
        std::string key = "read-key-" + std::to_string(i % 5);
        std::string value = system->read(key);
        if (!value.empty() && value == "read-value-" + std::to_string(i % 5)) {
            successfulReads++;
        }
        std::this_thread::sleep_for(300ms);
    }

    // Even with failures, we should get some successful reads
    EXPECT_GT(successfulReads, 5);
}

TEST_F(FaultToleranceTest, TestContinuousWriteDuringFailures) {
    // Start failure simulator
    system->startFailureSimulator(0.3, 0.3, 1);

    // Perform continuous writes
    int successfulWrites = 0;
    for (int i = 0; i < 30; i++) {
        bool success = system->write("continuous-key-" + std::to_string(i), 
                                    "continuous-value-" + std::to_string(i));
        if (success) {
            successfulWrites++;
        }
        std::this_thread::sleep_for(200ms);
    }

    // Master should never fail, so all writes should succeed
    EXPECT_EQ(30, successfulWrites);

    // Wait for recovery
    system->startFailureSimulator(0.0, 1.0, 1);
    std::this_thread::sleep_for(5s);

    // Check data store
    auto dataStore = system->getDataStore();
    EXPECT_FALSE(dataStore.empty());

    // Verify random sample of writes persisted
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 29);
    
    for (int i = 0; i < 5; i++) {
        int randomIndex = dist(gen);
        std::string expectedValue = "continuous-value-" + std::to_string(randomIndex);
        std::string actualValue = system->read("continuous-key-" + std::to_string(randomIndex));
        EXPECT_EQ(expectedValue, actualValue) << "Failed for index " << randomIndex;
    }
}