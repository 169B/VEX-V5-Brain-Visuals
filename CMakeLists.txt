cmake_minimum_required(VERSION 3.10)
project(VEXHostBrain VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

# Add HOST_MODE definition
add_definitions(-DHOST_MODE)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/liblvgl
)

# Collect source files
file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

# Create executable
add_executable(host_brain ${SOURCES})

# Link libraries
target_link_libraries(host_brain
    pthread
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(host_brain ws2_32)
endif()

# Output directory
set_target_properties(host_brain PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Custom target for running
add_custom_target(run
    COMMAND host_brain
    DEPENDS host_brain
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running host brain..."
)

# Custom target for UI installation
add_custom_target(ui-install
    COMMAND npm install
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/ui
    COMMENT "Installing UI dependencies..."
)

# Custom target for UI server
add_custom_target(ui-start
    COMMAND npm start
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/ui
    COMMENT "Starting UI server..."
)

# Print configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ flags: ${CMAKE_CXX_FLAGS}")
