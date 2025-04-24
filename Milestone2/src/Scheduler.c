#include "../include/Scheduler.h"
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
                Task *task = (Task *)queue_pop_head(sched->input_queues[0]);
                while (!task->run(task));
                queue_push_tail(sched->output_queue, task);
            }
            break;
        }
        case SCHED_RR: {
            if (!queue_is_empty(sched->input_queues[0])) {
                Task *task = (Task *)queue_pop_head(sched->input_queues[0]);
                int done = 0;
                for (int i = 0; i < sched->rr_quantum; i++) {
                    done = task->run(task);
                    if (done) break;
                }
                if (done)
                    queue_push_tail(sched->output_queue, task);
                else
                    queue_push_tail(sched->input_queues[0], task);
            }
            break;
        }
        case SCHED_MLFQ: {
            for (int level = 0; level < MLFQ_LEVELS; level++) {
                if (!queue_is_empty(sched->input_queues[level])) {
                    Task *task = (Task *)queue_pop_head(sched->input_queues[level]);
                    int done = 0;
                    for (int i = 0; i < sched->mlfq_quantum[level]; i++) {
                        done = task->run(task);
                        if (done) break;
                    }
                    if (done) {
                        queue_push_tail(sched->output_queue, task);
                    } else {
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
