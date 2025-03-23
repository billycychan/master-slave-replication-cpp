# Master-Slave Replication System with Fault Tolerance (C++)

This project implements a Master-Slave replication pattern with fault tolerance features inspired by the Raft consensus algorithm. The system runs a master node and three slave nodes that maintain synchronized key-value stores.

## System Architecture

The system consists of the following components:

1. **Node Interface**: Defines the common operations for both master and slave nodes.
2. **AbstractNode**: Base implementation of a node with common functionality.
3. **MasterNode**: Handles write operations and manages replication to slaves.
4. **SlaveNode**: Receives updates from the master and handles read operations.
5. **LogEntry**: Represents an entry in the replication log.
6. **ReplicationSystem**: Manages the entire cluster and provides a simple API.

![System Architecture](https://via.placeholder.com/800x400?text=Master-Slave+Replication+System+Architecture)

## Features

- **Asynchronous Replication**: Write operations are applied to the master immediately and then asynchronously replicated to slaves.
- **Fault Tolerance**: The system can handle node failures and recoveries.
- **Read-Write Separation**: Reads are distributed across slaves, while writes go to the master.
- **Log-Based Recovery**: When a slave comes back up, it recovers its state using the master's log.
- **Failure Simulation**: The system includes a failure simulator to demonstrate fault tolerance.

## Data Structure

The system uses C++ `std::map<std::string, std::string>` as the underlying data structure for storing key-value pairs.

## Log Entries

Each log entry includes:
- Log ID (monotonically increasing)
- Key and value of the data being updated
- Timestamp

## Prerequisites

- CMake 3.10 or higher
- C++17 compliant compiler (GCC 7+, Clang 5+, MSVC 2017+)
- pthread library

## Building and Running

### Using CMake

# Clone the repository
git clone https://github.com/yourusername/master-slave-replication-cpp.git
cd master-slave-replication-cpp

# Create build directory
```bash
mkdir -p build
cd build
```
# Generate build files
```
cmake .. -G "Unix Makefiles"
```
# Build the project
```
make
```

# Run the application
```bash
./replication_system
```
# How It Works

1. Write operations are sent to the master node.
2. The master creates a log entry and applies it to its local data store.
3. The log entry is asynchronously replicated to all slave nodes.
4. Read operations are randomly distributed across available slave nodes.
5. When a node fails, it's marked as down and excluded from operations.
6. When a node recovers, it requests missing log entries from the master.

# Fault Tolerance

The system implements fault tolerance through:
* **Asynchronous Replication**: Writes continue even if some slaves are down.
* **Log-Based Recovery**: Failed nodes can recover by requesting missing log entries.
* **Node Status Tracking**: The system keeps track of which nodes are up or down.

# Demonstration

The `main.cpp` file includes a demonstration that:
1. Initializes the system with sample data
2. Performs reads and writes
3. Simulates node failures and recoveries
4. Shows the final state of the data store

You can observe the system in action by running the executable and watching the console output.

# Interactive Mode

The system also includes an interactive mode that allows you to manually issue commands:
* `write <key> <value>`: Write a key-value pair to the master
* `read <key>`: Read a value from a random slave
* `show`: Display the current data store
* `exit`: Exit the program

To enable interactive mode, uncomment the `interactiveMode(system)` line in `main.cpp`.

# Implementation Notes

* Uses C++17 features like structured bindings and shared_ptr/enable_shared_from_this
* Implements thread-safety using mutexes and atomic operations
* Uses condition variables for interruptible waiting
* Handles cross-platform compatibility
