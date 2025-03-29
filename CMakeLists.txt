cmake_minimum_required(VERSION 3.14)
project(master-slave-replication VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Collect source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

# Create executable
add_executable(replication-system ${SOURCES})

# Set include directories for target
target_include_directories(replication-system PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Add threading support
find_package(Threads REQUIRED)
target_link_libraries(replication-system PRIVATE Threads::Threads)

# Installation
install(TARGETS replication-system DESTINATION bin)

# Enable testing
enable_testing()

# Add a simple test executable if needed
# add_executable(replication-tests tests/test_main.cpp)
# target_link_libraries(replication-tests PRIVATE ...)
# add_test(NAME replication-tests COMMAND replication-tests)

# Print configuration summary
message(STATUS "CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")