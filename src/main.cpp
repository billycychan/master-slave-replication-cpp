// src/main.cpp
#include <iostream>
#include <thread>
#include <chrono>
#include "system/ReplicationSystem.h"

/**
 * Demonstrates the replication system with sample data and operations.
 */
void demoSystem(ReplicationSystem& system) {
    try {
        // Initialize with some data
        std::cout << "\n--- Initializing data ---" << std::endl;
        system.write("key1", "value1");
        system.write("key2", "value2");
        system.write("key3", "value3");
        std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait for replication
        
        // Read from slaves
        std::cout << "\n--- Reading data from slaves ---" << std::endl;
        for (int i = 0; i < 5; i++) {
            std::string key = "key" + std::to_string((i % 3) + 1);
            std::string value = system.read(key);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        // Show all data
        std::cout << "\n--- Current data store ---" << std::endl;
        auto dataStore = system.getDataStore();
        for (const auto& [key, value] : dataStore) {
            std::cout << key << " = " << value << std::endl;
        }
        
        // Add more data
        std::cout << "\n--- Adding more data ---" << std::endl;
        system.write("key4", "value4");
        system.write("key5", "value5");
        std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait for replication
        
        // Read again
        std::cout << "\n--- Reading new data ---" << std::endl;
        for (int i = 0; i < 5; i++) {
            std::string key = "key" + std::to_string((i % 5) + 1);
            std::string value = system.read(key);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        // Update existing data
        std::cout << "\n--- Updating existing data ---" << std::endl;
        system.write("key1", "updated-value1");
        system.write("key3", "updated-value3");
        std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait for replication
        
        // Read after update
        std::cout << "\n--- Reading after updates ---" << std::endl;
        for (int i = 0; i < 5; i++) {
            std::string key = "key" + std::to_string((i % 5) + 1);
            std::string value = system.read(key);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        // Demonstrate failures and recovery
        std::cout << "\n--- Demonstrating failures and recovery (wait 30 seconds) ---" << std::endl;
        std::cout << "    Watch as nodes go down and come back up!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(30));
        
        // Show final state
        std::cout << "\n--- Final data store state ---" << std::endl;
        dataStore = system.getDataStore();
        for (const auto& [key, value] : dataStore) {
            std::cout << key << " = " << value << std::endl;
        }
        
        std::cout << "\nDemo completed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error in demo: " << e.what() << std::endl;
    }
}

/**
 * Interactive mode for manual testing of the replication system.
 */
void interactiveMode(ReplicationSystem& system) {
    std::string input;
    
    std::cout << "\n--- Interactive Mode ---" << std::endl;
    std::cout << "Commands: write <key> <value> | read <key> | show | exit" << std::endl;
    
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        if (input == "exit") {
            break;
        } else if (input == "show") {
            auto dataStore = system.getDataStore();
            for (const auto& [key, value] : dataStore) {
                std::cout << key << " = " << value << std::endl;
            }
        } else if (input.substr(0, 5) == "read ") {
            std::string key = input.substr(5);
            std::string value = system.read(key);
            if (value.empty()) {
                std::cout << "Key not found or all slaves are down" << std::endl;
            }
        } else if (input.substr(0, 6) == "write ") {
            std::string rest = input.substr(6);
            size_t pos = rest.find(' ');
            
            if (pos != std::string::npos) {
                std::string key = rest.substr(0, pos);
                std::string value = rest.substr(pos + 1);
                
                bool success = system.write(key, value);
                if (success) {
                    std::cout << "Write successful" << std::endl;
                } else {
                    std::cout << "Write failed (master down?)" << std::endl;
                }
            } else {
                std::cout << "Usage: write <key> <value>" << std::endl;
            }
        } else {
            std::cout << "Unknown command. Use write, read, show, or exit" << std::endl;
        }
    }
}

int main() {
    std::cout << "Starting Master-Slave Replication System with Fault Tolerance" << std::endl;
    
    // Create a replication system with 3 slaves
    ReplicationSystem system(3);
    
    // Start the failure simulator with moderate probabilities
    // 10% chance of failure, 30% chance of recovery per 5 seconds
    system.startFailureSimulator(0.1, 0.3, 5);
    
    // Demo the system
    try {
        demoSystem(system);
        
        // Uncomment to enable interactive mode
        // interactiveMode(system);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    // Shutdown the system properly
    system.shutdown();
    
    return 0;
}