cmake_minimum_required(VERSION 3.10)

# Project name
project(OperatingSystems C)

# Set the C standard
set(CMAKE_C_STANDARD 99)

# Set the source directory
set(SRC_DIR src)

# Add executable with the source files
add_executable(OperatingSystems
    ${SRC_DIR}/main.c
    ${SRC_DIR}/ThreadMetric.c
)

# Include directories for header files
target_include_directories(OperatingSystems PRIVATE ${SRC_DIR})
