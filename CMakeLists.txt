cmake_minimum_required(VERSION 3.10)
project(OperatingSystems C)  # Specify C language

set(CMAKE_C_STANDARD 11)  # Use C11 standard

# Set the source directory
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

# Find all source files in src/
file(GLOB SRC_FILES "${SRC_DIR}/*.c")

add_executable(OperatingSystems ${SRC_FILES})

# Include the src folder for headers
include_directories(${SRC_DIR})