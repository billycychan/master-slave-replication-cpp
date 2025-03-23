cmake_minimum_required(VERSION 3.10)
project(master_slave_replication VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Manually list source files (we'll update this as we add more files)
# Update SOURCES in CMakeLists.txt
set(SOURCES
    src/main.cpp
    src/model/LogEntry.cpp
    src/node/AbstractNode.cpp
    src/node/MasterNode.cpp
    src/node/SlaveNode.cpp
    src/system/ReplicationSystem.cpp
)
# Create executable
add_executable(replication_system ${SOURCES})

# Include directories
target_include_directories(replication_system PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Link with required libraries
target_link_libraries(replication_system PRIVATE pthread)