#ifndef OS_LOGIC_H
#define OS_LOGIC_H

#include <stdbool.h>
#include <semaphore.h>
#include "Queue.h"

#define CODE_SEGMENT_OFFSET 9
#define MLFQ_LEVELS 4
#define MEMORY_SIZE 60
#define MAX_PROCESSES 10

typedef enum { READY, RUNNING, BLOCKED, TERMINATED } ProcessState;
typedef enum { USER_INPUT, USER_OUTPUT, FILE_RW, NIL } Resource;
typedef enum { SCHED_FCFS, SCHED_ROUND_ROBIN, SCHED_MLFQ } SchedulerType;

typedef struct {
    int id;
    ProcessState state;
    int priority;
    int memory_start;
    int memory_end;
    int program_counter;
    Resource blockedResource;
    int timeslice_used;
    int arrivalTime;
    int time_in_queue;
    char filePath[256];
} ProcessControlBlock;

// Scheduler struct definition
typedef struct Scheduler {
    SchedulerType type;
    Queue* input_queues[MLFQ_LEVELS]; // For MLFQ, or input_queues[0] for FCFS/RR
    Queue* output_queue; // For completed processes or processes moving between queues
    int mlfq_quantum[MLFQ_LEVELS]; // Quantum for each MLFQ level
    int rr_quantum; // Quantum for Round Robin
    ProcessControlBlock* running_process;
} Scheduler;

typedef struct {
    char name[256];
    char data[256];
} MemoryWord;

typedef struct {
    MemoryWord memoryArray[MEMORY_SIZE];
} Memory;

// Simulation lifecycle
void os_init();
void os_reset();
void os_start(SchedulerType type, int quantum);
void os_step();
bool os_is_finished();

// Process management
ProcessControlBlock* os_add_process(const char* filepath, int arrival_time);
void os_terminate_process(ProcessControlBlock* process);

// Data access for GUI
typedef struct {
    int total_processes;
    int current_cycle;
    SchedulerType scheduler_type;
    ProcessControlBlock* running_process;
    // Add more fields as needed
} OSStatus;

void os_get_status(OSStatus* status);
void os_get_memory(Memory* mem);
void os_get_all_processes(ProcessControlBlock* arr[MAX_PROCESSES], int* count);
void os_get_ready_queue(Queue* queues[MLFQ_LEVELS]);
void os_get_blocked_queue(Queue* blocked);

// Utility
const char* os_process_state_str(ProcessState state);
const char* os_resource_str(Resource resource);

#endif // OS_LOGIC_H
