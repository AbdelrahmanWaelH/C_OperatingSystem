#include "../include/Scheduler.h"
#include "../include/Task.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h> 



// Example Usage for Scheduler :
// int main() {
//     Scheduler *sched = scheduler_new(SCHED_RR,2); // Change to SCHED_FCFS or SCHED_RR or SCHED_MLFQ

//     Task *t1 = malloc(sizeof(Task));
//     *t1 = (Task){1, "Task-A", 0, 5,0, task_run};

//     Task *t2 = malloc(sizeof(Task));
//     *t2 = (Task){2, "Task-B", 0, 3, 0,task_run};

//     Task *t3 = malloc(sizeof(Task));
//     *t3 = (Task){3, "Task-C", 0, 10,0, task_run};

//     scheduler_add_task(sched, t1);
//     scheduler_add_task(sched, t2);
//     scheduler_add_task(sched, t3);

//     for (int i = 0; i < 30; i++) {
//         printf("=== Cycle %d ===\n", i);
//         scheduler_step(sched);
//         Task *done = scheduler_fetch_output(sched);
//         if (done)
//             printf("[Finished] %s (PID %d)\n", done->name, done->pid);
//     }

//     scheduler_free(sched);
//     free(t1); free(t2); free(t3);
//     return 0;
// }
