#include "../include/os_logic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include "Queue.h"
#include "FileReader.h"
#include "Semaphores.h" // Ensure this is included

// ========== Global Simulation State =============
Memory mainMemory;
Queue* blockedQueue = NULL; // Changed from 'Queue blockedQueue;'
ProcessControlBlock* userInputBlockingProcess = NULL;
ProcessControlBlock* userOutputBlockingProcess = NULL;
ProcessControlBlock* fileBlockingProcess = NULL;
int processIdCounter = 0;
SchedulerType current_scheduler_type = SCHED_FCFS;
Scheduler* scheduler = NULL;
int current_cycle_count = 0;
ProcessControlBlock* loaded_processes_sim[MAX_PROCESSES];
int num_loaded_processes_sim = 0;
Queue* loadingQueue = NULL;

// ========== Utility Functions =============
const char* os_process_state_str(ProcessState state) {
    switch (state) {
        case READY: return "READY";
        case RUNNING: return "RUNNING";
        case BLOCKED: return "BLOCKED";
        case TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}
const char* os_resource_str(Resource resource) {
    switch (resource) {
        case USER_INPUT: return "userInput";
        case USER_OUTPUT: return "userOutput";
        case FILE_RW: return "file";
        case NIL: return "None";
        default: return "Unknown";
    }
}

// ========== Simulation Lifecycle =============
// Helper function to free a Scheduler
static void free_scheduler(Scheduler* sched) {
    if (!sched) return;
    for (int i = 0; i < MLFQ_LEVELS; i++) {
        if (sched->input_queues[i]) {
            queue_free(sched->input_queues[i]);
            sched->input_queues[i] = NULL;
        }
    }
    if (sched->output_queue) {
        queue_free(sched->output_queue);
        sched->output_queue = NULL;
    }
    free(sched);
}

void os_init() {
    initSemaphores(); // Call to initialize semaphores
    memset(&mainMemory, 0, sizeof(Memory));
    for (int i = 0; i < MEMORY_SIZE; i++) {
        mainMemory.memoryArray[i].name[0] = '\0';
        mainMemory.memoryArray[i].data[0] = '\0';
    }

    // Old problematic lines:
    // queue_free(&blockedQueue);
    // blockedQueue = *queue_new();

    // Corrected handling for blockedQueue as a pointer:
    if (blockedQueue) {
        queue_free(blockedQueue);
        // blockedQueue = NULL; // Not strictly necessary before reassignment by queue_new()
    }
    blockedQueue = queue_new();

    // sem_init for userInputSemaphore, userOutputSemaphore, fileSemaphore are correctly in Semaphores.c's initSemaphores
    // Ensure initSemaphores() is called if not already. Assuming it is.
    userInputBlockingProcess = NULL;
    userOutputBlockingProcess = NULL;
    fileBlockingProcess = NULL;
    processIdCounter = 0;
    current_cycle_count = 0;
    num_loaded_processes_sim = 0;
    if (loadingQueue) queue_free(loadingQueue);
    loadingQueue = queue_new();
    if (scheduler) {
        free_scheduler(scheduler);
        scheduler = NULL;
    }
}
void os_reset() { os_init(); }

void os_start(SchedulerType type, int quantum) {
    if (scheduler) {
        free_scheduler(scheduler);
        scheduler = NULL;
    }
    scheduler = (Scheduler*) malloc(sizeof(Scheduler));
    scheduler->type = type;
    scheduler->rr_quantum = (type == SCHED_ROUND_ROBIN) ? quantum : 0;
    for (int i = 0; i < MLFQ_LEVELS; i++) scheduler->input_queues[i] = queue_new();
    scheduler->output_queue = queue_new();
    int base = 1;
    for (int i = 0; i < MLFQ_LEVELS; i++) scheduler->mlfq_quantum[i] = base << i;
    scheduler->running_process = NULL;
    current_scheduler_type = type;
    // Add all loaded processes to scheduler
    for (int i = 0; i < num_loaded_processes_sim; i++) {
        queue_push_tail(scheduler->input_queues[0], loaded_processes_sim[i]);
    }
}

void os_step() {
    current_cycle_count++;
    // TODO: Implement scheduler_step and process execution logic
    // This is a stub for now
}

bool os_is_finished() {
    for (int i = 0; i < num_loaded_processes_sim; i++) {
        if (loaded_processes_sim[i]->state != TERMINATED) return false;
    }
    return true;
}

// ========== Process Management =============
ProcessControlBlock* os_add_process(const char* filepath, int arrival_time) {
    if (num_loaded_processes_sim >= MAX_PROCESSES) return NULL;
    ProcessControlBlock* pcb = (ProcessControlBlock*)malloc(sizeof(ProcessControlBlock));
    pcb->id = processIdCounter++;
    pcb->state = READY;
    pcb->priority = 0;
    pcb->memory_start = 0; // TODO: Find free memory
    pcb->memory_end = 0;   // TODO: Set correct end
    pcb->program_counter = 0;
    pcb->blockedResource = NIL;
    pcb->timeslice_used = 0;
    pcb->arrivalTime = arrival_time;
    pcb->time_in_queue = 0;
    strncpy(pcb->filePath, filepath, 255);
    loaded_processes_sim[num_loaded_processes_sim++] = pcb;
    return pcb;
}
void os_terminate_process(ProcessControlBlock* process) {
    if (!process) return;
    process->state = TERMINATED;
    // TODO: Free memory, remove from queues, etc.
}

// ========== Data Access for GUI =============
void os_get_status(OSStatus* status) {
    status->total_processes = num_loaded_processes_sim;
    status->current_cycle = current_cycle_count;
    status->scheduler_type = current_scheduler_type;
    status->running_process = scheduler ? scheduler->running_process : NULL;
}
void os_get_memory(Memory* mem) {
    memcpy(mem, &mainMemory, sizeof(Memory));
}
void os_get_all_processes(ProcessControlBlock* arr[MAX_PROCESSES], int* count) {
    for (int i = 0; i < num_loaded_processes_sim; i++) arr[i] = loaded_processes_sim[i];
    *count = num_loaded_processes_sim;
}
void os_get_ready_queue(Queue* queues[MLFQ_LEVELS]) {
    if (!scheduler) return;
    for (int i = 0; i < MLFQ_LEVELS; i++) queues[i] = scheduler->input_queues[i];
}
void os_get_blocked_queue(Queue* blocked) {
    *blocked = *blockedQueue;
}
