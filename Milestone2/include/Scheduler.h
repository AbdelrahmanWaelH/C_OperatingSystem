#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "Queue.h"
#include "Task.h"

#define MLFQ_LEVELS 4

typedef enum { SCHED_FCFS, SCHED_RR, SCHED_MLFQ } SchedulerType;

typedef struct Scheduler {
    SchedulerType type;
    Queue *input_queues[MLFQ_LEVELS];
    Queue *output_queue;
    int mlfq_quantum[MLFQ_LEVELS];
    int rr_quantum;
} Scheduler;

Scheduler *scheduler_new(SchedulerType type, ...);
void scheduler_add_task(Scheduler *sched, Task *task);
void scheduler_step(Scheduler *sched);
Task *scheduler_fetch_output(Scheduler *sched);
void scheduler_free(Scheduler *sched);

#endif
