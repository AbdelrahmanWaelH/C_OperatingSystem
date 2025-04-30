#include <stdio.h>
#include "Task.h"

int task_run(Task *task) {
    task->runtime++;
    printf("Running %s (PID %d), runtime: %d/%d\n", task->name, task->pid, task->runtime, task->total_required);
    return task->runtime >= task->total_required;
}