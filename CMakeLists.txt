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

# Exclude test files from main executable
list(FILTER SOURCES EXCLUDE REGEX ".*tests/.*\.cpp$")

# Create executable
add_executable(replication-system ${SOURCES})

# Set include directories for target
target_include_directories(replication-system PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Add threading support
find_package(Threads REQUIRED)
target_link_libraries(replication-system PRIVATE Threads::Threads)

# Installation
install(TARGETS replication-system DESTINATION bin)

# Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Get all source files except main.cpp and test files
file(GLOB_RECURSE LIB_SOURCES 
    "src/*.cpp"
)
list(FILTER LIB_SOURCES EXCLUDE REGEX ".*tests/.*\.cpp$")
list(FILTER LIB_SOURCES EXCLUDE REGEX "src/main\.cpp$")

# Add test executable
add_executable(replication-tests 
  src/tests/NodeTest.cpp
  src/tests/MainTest.cpp
  src/tests/FaultToleranceTest.cpp
  ${LIB_SOURCES}
)

# Link against gtest and our main library code
target_link_libraries(replication-tests
  PRIVATE
  gtest
  gtest_main
  Threads::Threads
)

# Include directories for tests
target_include_directories(replication-tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(replication-tests)

# Print configuration summary
message(STATUS "CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")