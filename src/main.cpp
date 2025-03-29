#include "system/ReplicationSystem.h"
#include "model/LogEntry.h"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <sstream>
#include <vector>
#include <map>
#include <ctime>
#include <iomanip>

using namespace replication;
using namespace std::chrono_literals;

// Forward declarations
void demoSystem(system::ReplicationSystem& system);
void interactiveMode(system::ReplicationSystem& system);
std::vector<std::string> splitString(const std::string& input, char delimiter);

int main(int argc, char* argv[]) {
    std::cout << "Starting Master-Slave Replication System with Fault Tolerance" << std::endl;
    
    // Create a replication system with 3 slaves
    system::ReplicationSystem system(3);
    
    // Start the failure simulator with moderate probabilities
    // 10% chance of failure, 30% chance of recovery per 5 seconds
    system.startFailureSimulator(0.1, 0.3, 5);
    
    // Check if we should run in demo mode
    if (argc > 1 && std::string(argv[1]) == "--demo") {
        try {
            demoSystem(system);
        } catch (const std::exception& e) {
            std::cerr << "Exception during demo: " << e.what() << std::endl;
        }
    } else {
        // Run in interactive mode
        interactiveMode(system);
    }
    
    return 0;
}

void demoSystem(system::ReplicationSystem& system) {
    // Initialize with some data
    std::cout << "\n--- Initializing data ---" << std::endl;
    system.write("key1", "value1");
    system.write("key2", "value2");
    system.write("key3", "value3");
    std::this_thread::sleep_for(2s); // Wait for replication
    
    // Read from slaves
    std::cout << "\n--- Reading data from slaves ---" << std::endl;
    for (int i = 0; i < 5; i++) {
        std::string key = "key" + std::to_string((i % 3) + 1);
        std::string value = system.read(key);
        std::this_thread::sleep_for(500ms);
    }
    
    // Show all data
    std::cout << "\n--- Current data store ---" << std::endl;
    auto dataStore = system.getDataStore();
    if (!dataStore.empty()) {
        for (const auto& [key, value] : dataStore) {
            std::cout << key << " = " << value << std::endl;
        }
    }
    
    // Add more data
    std::cout << "\n--- Adding more data ---" << std::endl;
    system.write("key4", "value4");
    system.write("key5", "value5");
    std::this_thread::sleep_for(2s); // Wait for replication
    
    // Read again
    std::cout << "\n--- Reading new data ---" << std::endl;
    for (int i = 0; i < 5; i++) {
        std::string key = "key" + std::to_string((i % 5) + 1);
        std::string value = system.read(key);
        std::this_thread::sleep_for(300ms);
    }
    
    // Update existing data
    std::cout << "\n--- Updating existing data ---" << std::endl;
    system.write("key1", "updated-value1");
    system.write("key3", "updated-value3");
    std::this_thread::sleep_for(2s); // Wait for replication
    
    // Read after update
    std::cout << "\n--- Reading after updates ---" << std::endl;
    for (int i = 0; i < 5; i++) {
        std::string key = "key" + std::to_string((i % 5) + 1);
        std::string value = system.read(key);
        std::this_thread::sleep_for(300ms);
    }
    
    // Demonstrate delete operation
    std::cout << "\n--- Demonstrating delete operation ---" << std::endl;
    system.deleteKey("key2");
    system.deleteKey("key4");
    std::this_thread::sleep_for(2s); // Wait for replication
    
    // Read after delete
    std::cout << "\n--- Reading after deletes ---" << std::endl;
    for (int i = 0; i < 5; i++) {
        std::string key = "key" + std::to_string((i % 5) + 1);
        std::string value = system.read(key);
        std::cout << "Key: " << key << ", Value: " << (value.empty() ? "<deleted>" : value) << std::endl;
        std::this_thread::sleep_for(300ms);
    }
    
    // Demonstrate failures and recovery
    std::cout << "\n--- Demonstrating failures and recovery (wait 30 seconds) ---" << std::endl;
    std::cout << "    Watch as nodes go down and come back up!" << std::endl;
    std::this_thread::sleep_for(30s);
    
    // Show final state
    std::cout << "\n--- Final data store state ---" << std::endl;
    dataStore = system.getDataStore();
    if (!dataStore.empty()) {
        for (const auto& [key, value] : dataStore) {
            std::cout << key << " = " << value << std::endl;
        }
    }
    
    std::cout << "\nDemo completed!" << std::endl;
    system.shutdown();
}

void interactiveMode(system::ReplicationSystem& system) {
    std::string input;
    
    std::cout << "\n--- Interactive Mode ---" << std::endl;
    std::cout << "Commands: write <key> <value> | read <key> | delete <key> | show | logs | status | exit" << std::endl;
    
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        if (input == "exit") {
            break;
        } else if (input == "show") {
            auto dataStore = system.getDataStore();
            std::cout << "\n--- Current Data Store ---" << std::endl;
            if (dataStore.empty()) {
                std::cout << "(empty)" << std::endl;
            } else {
                for (const auto& [key, value] : dataStore) {
                    std::cout << key << " = " << value << std::endl;
                }
            }
        } else if (input == "logs") {
            auto logs = system.getLogs();
            std::cout << "\n--- Replication Log Entries ---" << std::endl;
            if (logs.empty()) {
                std::cout << "(no log entries)" << std::endl;
            } else {
                for (const auto& entry : logs) {
                    std::string operationStr = entry.isDelete() ? "DELETE" : "WRITE";
                    
                    // Format timestamp
                    auto timeMs = static_cast<time_t>(entry.getTimestamp() / 1000);
                    std::tm* tm = std::localtime(&timeMs);
                    std::stringstream timeStr;
                    timeStr << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
                    
                    std::cout << "Log #" << entry.getId() << ": " << operationStr 
                              << " key='" << entry.getKey() << "'"
                              << (entry.isDelete() ? "" : " value='" + entry.getValue() + "'")
                              << " (" << timeStr.str() << ")" << std::endl;
                }
            }
        } else if (input == "status") {
            auto nodeStatus = system.getNodesStatus();
            std::cout << "\n--- Node Status ---" << std::endl;
            for (const auto& [nodeId, isUp] : nodeStatus) {
                std::cout << nodeId << ": " << (isUp ? "UP" : "DOWN") << std::endl;
            }
        } else if (input.compare(0, 5, "read ") == 0) {
            std::string key = input.substr(5);
            std::string value = system.read(key);
            if (value.empty()) {
                std::cout << "Key not found or all slaves are down" << std::endl;
            }
        } else if (input.compare(0, 7, "delete ") == 0) {
            std::string key = input.substr(7);
            bool success = system.deleteKey(key);
            if (success) {
                std::cout << "Delete successful" << std::endl;
            } else {
                std::cout << "Delete failed (key not found or master down)" << std::endl;
            }
        } else if (input.compare(0, 6, "write ") == 0) {
            std::string command = input.substr(6);
            auto parts = splitString(command, ' ');
            
            if (parts.size() >= 2) {
                std::string key = parts[0];
                std::string value = parts[1];
                // If there are more parts, join them with spaces
                for (size_t i = 2; i < parts.size(); i++) {
                    value += " " + parts[i];
                }
                
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
            std::cout << "Unknown command. Use write, read, delete, show, logs, status, or exit" << std::endl;
        }
    }
    
    system.shutdown();
}

std::vector<std::string> splitString(const std::string& input, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(input);
    
    while (std::getline(tokenStream, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}