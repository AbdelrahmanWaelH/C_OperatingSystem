cmake_minimum_required(VERSION 3.10)

project(Milestone2OS C)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_STANDARD 99)

include_directories(${PROJECT_SOURCE_DIR}/../include)# Add source files
add_executable(Milestone2OS
    main.c
    Queue.c
    FileReader.c
    Semaphores.c
    os_logic.c
)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

target_link_libraries(Milestone2OS ${GTK3_LIBRARIES})