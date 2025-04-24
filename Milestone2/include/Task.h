#ifndef TASK_H
#define TASK_H

typedef struct Task {
    int pid;
    const char *name;
    int runtime;
    int total_required;
    int (*run)(struct Task *task);
} Task;

int task_run(Task *task);

#endif