cmake_minimum_required(VERSION 3.10)

project(Milestone2OS C)

set(CMAKE_C_STANDARD 99)

include_directories(${PROJECT_SOURCE_DIR}/../include)# Add source files
add_executable(Milestone2OS
    main.c
    Queue.c
    FileReader.c
    Scheduler.c
    SchedulerTest.c
    Task.c
)