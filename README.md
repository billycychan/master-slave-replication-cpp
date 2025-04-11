# Master-Slave Replication System with Fault Tolerance (C++)

This project implements a Master-Slave replication pattern with fault tolerance features inspired by the Raft consensus algorithm. The system runs a master node and three slave nodes within a single Java application, which can be containerized using Docker.

## System Architecture

The system consists of the following components:

1. **Node Interface**: Defines the common operations for both master and slave nodes.
2. **AbstractNode**: Base implementation of a node with common functionality.
3. **MasterNode**: Handles write operations and manages replication to slaves.
4. **SlaveNode**: Receives updates from the master and handles read operations.
5. **LogEntry**: Represents an entry in the replication log.
6. **ReplicationSystem**: Manages the entire cluster and provides a simple API.

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
```
Log #1: WRITE key='user1' value='John' (2025-03-29 19:45:12)
```

## Building and Running

### Prerequisites

- CMake 3.10 or higher
- C++17 compliant compiler (GCC 7+, Clang 5+, MSVC 2017+)
- pthread library


### Using CMake

```bash
# Clone the repository
git clone https://github.com/billycychan/master-slave-replication-cpp.git
cd master-slave-replication-cpp

# Create build directory
mkdir -p build
cd build

# Generate build files
cmake ..

# Build the project
make

# Run the application
./replication-system
```

### Running Tests

The project includes a comprehensive test suite that verifies the functionality of the replication system, including fault tolerance features. To run the tests:

```bash
# From the build directory
./replication-tests
```

The test suite includes:
- **NodeTest**: Tests basic node operations, master-slave communication, and node failure/recovery scenarios
- **MainTest**: Tests the main replication system API and data consistency
- **FaultToleranceTest**: Tests the system's ability to handle node failures during operation


## How It Works

1. Write and delete operations are sent to the master node.
2. The master creates a log entry with the appropriate operation type and applies it to its local data store.
3. The log entry is asynchronously replicated to all slave nodes.
4. Read operations are randomly distributed across available slave nodes.
5. When a node fails, it's marked as down and excluded from operations.
6. When a node recovers, it requests missing log entries from the master and applies them according to their operation type.

## Fault Tolerance

The system implements fault tolerance through:
- **Asynchronous Replication**: Writes continue even if some slaves are down.
- **Log-Based Recovery**: Failed nodes can recover by requesting missing log entries.
- **Node Status Tracking**: The system keeps track of which nodes are up or down.





## Demonstration 

The system includes a comprehensive demonstration mode that showcases all the key features. To run in demo mode, use the `--demo` flag when starting the application:

```bash
./replication-system --demo
```

The demonstration will automatically perform the following sequence of operations:

1. **Initial Data Setup**:
   - Writes three key-value pairs (`key1=value1`, `key2=value2`, `key3=value3`)
   - Waits for replication to complete across all available slave nodes

2. **Read Operations**:
   - Performs multiple read operations from slave nodes
   - Displays the current contents of the data store

3. **Data Modifications**:
   - Adds additional data (`key4=value4`, `key5=value5`)
   - Updates existing values (`key1=updated-value1`, `key3=updated-value3`)
   - Demonstrates read operations after each modification

4. **Delete Operations**:
   - Removes specific keys from the data store (`key2`, `key4`)
   - Shows read operations after deletion, displaying `<deleted>` for removed keys

5. **Fault Tolerance Demonstration**:
   - Runs for 30 seconds to showcase node failures and automatic recovery
   - During this time, you'll see nodes randomly going down and coming back up
   - The system continues to function despite node failures

6. **Final State**:
   - Displays the final state of the data store after all operations
   - Shows how the system maintained data consistency throughout the process

This demonstration provides a visual representation of how the replication system handles various operations and maintains fault tolerance in the face of node failures.

## Interactive Mode

The system includes an interactive mode that allows you to manually issue commands and observe the system's behavior. Interactive mode is the default when running the application without any arguments. To run in demo mode instead, use the `--demo` flag.

### Available Commands

* `write <key> <value>`: Write a key-value pair to the master node
* `read <key>`: Read a value from a random slave node
* `delete <key>`: Delete a key-value pair from the system
* `show`: Display the current contents of the data store
* `logs`: Display all log entries in the replication log
* `status`: Show the current status (UP/DOWN) of all nodes
* `exit`: Shut down the system and exit the program

### Example Session

```
> write user1 John Doe
Write successful
> write user2 Jane Smith
Write successful
> read user1
John Doe
> show
--- Current Data Store ---
user1 = John Doe
user2 = Jane Smith
> status
--- Node Status ---
master: UP
slave-1: UP
slave-2: DOWN
slave-3: UP
> delete user1
Delete successful
> show
--- Current Data Store ---
user2 = Jane Smith
> exit
```

## Project Structure

```
master-slave-replication-cpp/
├── .gitignore                  # Git ignore rules
├── CMakeLists.txt              # CMake build configuration
├── diagrams.md                 # System architecture diagrams (Mermaid)
├── README.md                   # Project documentation
├── Test_Plan.md                # Test plan description
├── Test_report.md              # Test report summary
└── src/                        # Source code
    ├── main.cpp                # Main application entry point
    ├── model/                  # Data model definitions
    │   ├── LogEntry.cpp        # Log entry implementation
    │   └── LogEntry.h          # Log entry interface
    ├── node/                   # Node implementations (master/slave)
    │   ├── AbstractNode.cpp
    │   ├── AbstractNode.h
    │   ├── MasterNode.cpp
    │   ├── MasterNode.h
    │   ├── Node.h              # Node interface
    │   ├── SlaveNode.cpp
    │   └── SlaveNode.h
    ├── system/                 # Core system logic
    │   ├── ReplicationSystem.cpp
    │   └── ReplicationSystem.h
    └── tests/                  # Unit test suite
        ├── FaultToleranceTest.cpp
        ├── MainTest.cpp
        └── NodeTest.cpp

```



## System Diagrams

Detailed design diagrams, including class diagrams, state charts, flowcharts, and sequence diagrams, are provided in a separate file:

[Click here to view full system diagrams](./diagrams.md)

These diagrams illustrate:

- Class relationships and system structure
- Master-slave replication sequence
- Fault-tolerant recovery flows
- Data flow across nodes and components
- State transitions during normal and failure operation


## Implementation Notes

* Uses C++17 features like structured bindings and shared_ptr/enable_shared_from_this
* Implements thread-safety using mutexes and atomic operations
* Uses condition variables for interruptible waiting
* Handles cross-platform compatibility

## License

This project is licensed under the [MIT License](https://opensource.org/licenses/MIT).

