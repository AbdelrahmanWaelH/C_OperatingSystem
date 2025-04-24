#include "../include/Scheduler.h"
#include "../include/Semaphores.h"
#include <stdlib.h>
#include <stdarg.h>

Scheduler *scheduler_new(SchedulerType type, ...) {
    Scheduler *s = (Scheduler *)malloc(sizeof(Scheduler));
    s->type = type;
    va_list args;
    va_start(args, type);
    if (type == SCHED_RR)
        s->rr_quantum = va_arg(args, int);
    else
        s->rr_quantum = 0;
    va_end(args);

    for (int i = 0; i < MLFQ_LEVELS; i++)
        s->input_queues[i] = queue_new();
    s->output_queue = queue_new();
    int base = 1;
    for (int i = 0; i < MLFQ_LEVELS; i++)
        s->mlfq_quantum[i] = base << i; // 1, 2, 4, 8
    return s;

}

//task->run <==> executeSingleLinePCB(pcb)

void scheduler_add_task(Scheduler *sched, Task *task) {
    if (sched->type == SCHED_MLFQ)
        queue_push_tail(sched->input_queues[0], task);
    else
        queue_push_tail(sched->input_queues[0], task);
}

void scheduler_step(Scheduler *sched) {
    switch (sched->type) {
        case SCHED_FCFS: {
            if (!queue_is_empty(sched->input_queues[0])) {
                Task *task = (Task *)queue_peek_head(sched->input_queues[0]);
                int done = task->run(task);
                if (done) {
                    queue_pop_head(sched->input_queues[0]);
                    queue_push_tail(sched->output_queue, task);
                }
            }
            break;
        }

        case SCHED_RR: {
            if (!queue_is_empty(sched->input_queues[0])) {
                Task *task = (Task *)queue_peek_head(sched->input_queues[0]);
                int done = task->run(task);
                task->timeslice_used++;
                if (done) {
                    queue_pop_head(sched->input_queues[0]);
                    queue_push_tail(sched->output_queue, task);
                } else if (task->timeslice_used >= sched->rr_quantum) {
                    task->timeslice_used = 0;
                    queue_pop_head(sched->input_queues[0]);
                    queue_push_tail(sched->input_queues[0], task);
                }
            }
            break;
        }

        case SCHED_MLFQ: {
            for (int level = 0; level < MLFQ_LEVELS; level++) {
                if (!queue_is_empty(sched->input_queues[level])) {
                    Task *task = (Task *)queue_peek_head(sched->input_queues[level]);
                    
                    int done = task->run(task);
                    task->timeslice_used++;
                                        
                    if (done) {
                        task = (Task *)queue_pop_head(sched->input_queues[level]);
                        task->timeslice_used = 0;
                        queue_push_tail(sched->output_queue, task);
                    } else if (task->timeslice_used >= sched->mlfq_quantum[level]) {
                        task = (Task *)queue_pop_head(sched->input_queues[level]);
                        task->timeslice_used = 0;
                        int next_level = (level < MLFQ_LEVELS - 1) ? level + 1 : level;
                        queue_push_tail(sched->input_queues[next_level], task);
                    }       
                    break;
                }
            }
            break;
        }
    }
}


Task *scheduler_fetch_output(Scheduler *sched) {
    return (Task *)queue_pop_head(sched->output_queue);
}

void scheduler_free(Scheduler *sched) {
    for (int i = 0; i < MLFQ_LEVELS; i++)
        queue_free(sched->input_queues[i]);
    queue_free(sched->output_queue);
    free(sched);
}


void is_free(){

    int val;
    sem_getvalue(&userInputSemaphore, &val);

    if(val == 0); // not free
    else ; //free

}