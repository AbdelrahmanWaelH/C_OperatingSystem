#define _GNU_SOURCE
#include "ThreadMetric.h"
#include "TimeUtils.h"
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <malloc.h>

// Structs to store details about each thread's performance metrics
// These are global so that they can be accessed from any method
ThreadMetric threadMetric1;
ThreadMetric threadMetric2;
ThreadMetric threadMetric3;

// Define the schedule policy either First In First Out Or Round Robin
// 1 -> FIFO , 2 -> RR
int SCHEDULE_POLICY;

long get_memory_usage_KB(){
    struct rusage usage;    
    // Get resource usage for current thread
    getrusage(RUSAGE_THREAD, &usage);
    
    // Print maximum resident set size (RSS) in KB
    return usage.ru_maxrss;
}

void *inBetweenChars(void *arg)
{ // Thread 1 start

    struct timespec inputWaitStart = {0};
    struct timespec inputWaitEnd = {0};
    struct timespec timeDifference = {0};
    struct timespec totalWaitTime = {0};
    struct timespec temp = {0};

    // Record thread start time.
    clock_gettime(CLOCK_MONOTONIC, &threadMetric1.startTime);

    char start, end;

    printf("Enter the start character: \n");
    clock_gettime(CLOCK_MONOTONIC, &inputWaitStart);
    scanf(" %c", &start);
    clock_gettime(CLOCK_MONOTONIC, &inputWaitEnd);

    subtractTimespec(&timeDifference, &inputWaitEnd, &inputWaitStart);
    addTimespec(&totalWaitTime, &totalWaitTime, &timeDifference);

    printf("Enter the end character: \n");
    clock_gettime(CLOCK_MONOTONIC, &inputWaitStart);
    scanf(" %c", &end);
    clock_gettime(CLOCK_MONOTONIC, &inputWaitEnd);

    subtractTimespec(&timeDifference, &inputWaitEnd, &inputWaitStart);
    addTimespec(&totalWaitTime, &totalWaitTime, &timeDifference);

    /*if(start > end) swap(start, end)*/

    while (start <= end)
    {
        printf("%c\n", start);
        start++;
    }

    clock_gettime(CLOCK_MONOTONIC, &threadMetric1.finishTime);
    threadMetric1.waitTime = timespecToMillis(totalWaitTime);
    subtractTimespec(&temp, &threadMetric1.finishTime, &threadMetric1.releaseTime);
    threadMetric1.turnaroundTime = timespecToMillis(temp);
    threadMetric1.executionTime = threadMetric1.turnaroundTime - threadMetric1.waitTime;
    threadMetric1.responseTime = timespecToMillis(threadMetric1.startTime) - timespecToMillis(threadMetric1.releaseTime);
    threadMetric1.cpuUsage = threadMetric1.executionTime / (threadMetric1.executionTime + threadMetric1.waitTime);
    threadMetric1.memoryUsage = get_memory_usage_KB();

} // Thread 1 end

void *sumAvgProduct(void *arg)
{ // Thread 3 start

    struct timespec inputWaitStart = {0};
    struct timespec inputWaitEnd = {0};
    struct timespec timeDifference = {0};
    struct timespec totalWaitTime = {0};
    struct timespec temp = {0};

    clock_gettime(CLOCK_MONOTONIC, &threadMetric3.startTime);

    int start, end;
    int sum = 0;
    long long product = 1;
    float average = 0;

    printf("Enter the start integer: \n");
    clock_gettime(CLOCK_MONOTONIC, &inputWaitStart);
    scanf(" %d", &start);
    clock_gettime(CLOCK_MONOTONIC, &inputWaitEnd);

    subtractTimespec(&timeDifference, &inputWaitEnd, &inputWaitStart);
    addTimespec(&totalWaitTime, &totalWaitTime, &timeDifference);

    printf("Enter the end integer: \n");
    clock_gettime(CLOCK_MONOTONIC, &inputWaitStart);
    scanf(" %d", &end);
    clock_gettime(CLOCK_MONOTONIC, &inputWaitEnd);

    subtractTimespec(&timeDifference, &inputWaitEnd, &inputWaitStart);
    addTimespec(&totalWaitTime, &totalWaitTime, &timeDifference);

    /*if(start > end) swap(start, end)*/

    int delta = end - start + 1;
    while (start <= end)
    {
        sum += start;
        product *= start;
        start++;
    }
    average = (float)sum / delta;

    printf("\nThe sum is : %d", sum);
    printf("\nThe product is : %lld", product);
    printf("\nThe average is : %.2f\n", average);

    clock_gettime(CLOCK_MONOTONIC, &threadMetric3.finishTime);
    threadMetric3.waitTime = timespecToMillis(totalWaitTime);
    subtractTimespec(&temp, &threadMetric3.finishTime, &threadMetric3.releaseTime);
    threadMetric3.turnaroundTime = timespecToMillis(temp);
    threadMetric3.executionTime = threadMetric3.turnaroundTime - threadMetric3.waitTime;
    threadMetric3.responseTime = timespecToMillis(threadMetric3.startTime) - timespecToMillis(threadMetric3.releaseTime);
    threadMetric3.cpuUsage = threadMetric3.executionTime / (threadMetric3.executionTime + threadMetric3.waitTime);
    threadMetric3.memoryUsage = get_memory_usage_KB();

} // Thread 3 end

void *functionPrint(void *arg)
{ // Thread 2 starts
    struct timespec inputWaitStart = {0};
    struct timespec inputWaitEnd = {0};
    struct timespec timeDifference = {0};
    struct timespec totalWaitTime = {0};
    struct timespec temp = {0};

    clock_gettime(CLOCK_MONOTONIC, &threadMetric2.startTime);

    pthread_t threadId = pthread_self();

    printf("##############\nWelcome to the printing park!\n");

    printf("Brought to you by thread with id : %lu \n", threadId);

    printf("Enjoy your stay!\n#################\n");

    clock_gettime(CLOCK_MONOTONIC, &threadMetric2.finishTime);
    threadMetric2.waitTime = timespecToMillis(totalWaitTime);
    subtractTimespec(&temp, &threadMetric2.finishTime, &threadMetric2.releaseTime);
    threadMetric2.turnaroundTime = timespecToMillis(temp);
    threadMetric2.executionTime = threadMetric2.turnaroundTime - threadMetric2.waitTime;
    threadMetric2.responseTime = timespecToMillis(threadMetric2.startTime) - timespecToMillis(threadMetric2.releaseTime);
    threadMetric2.cpuUsage = threadMetric2.executionTime / (threadMetric2.executionTime + threadMetric2.waitTime);
    threadMetric2.memoryUsage = get_memory_usage_KB();

}// Thread 2 ends

/*void testMethod()*/
/*{*/
/**/
/*    struct timespec startTime, finishTime, executionStart, executionEnd;*/
/*    clock_gettime(CLOCK_MONOTONIC, &startTime);*/
/**/
/*    clock_gettime(CLOCK_MONOTONIC, &executionStart);*/
/*    int x = 0;*/
/*    for (long i = 0; i < 10e9; i++)*/
/*    {*/
/*        x++;*/
/*    }*/
/*    clock_gettime(CLOCK_MONOTONIC, &executionEnd);*/
/**/
/*    sleep(2);*/
/*    clock_gettime(CLOCK_MONOTONIC, &finishTime);*/
/**/
/*    double startMS, finishMS, execStartMS, execEndMS, executionMS;*/
/*    startMS = timespecToMillis(startTime);*/
/*    finishMS = timespecToMillis(finishTime);*/
/*    execStartMS = timespecToMillis(executionStart);*/
/*    execEndMS = timespecToMillis(executionEnd);*/
/**/
/*    executionMS = execEndMS - execStartMS;*/
/**/
/*    printf("CPU Utilization = %lf%%\n", 100 * (executionMS) / (finishMS - startMS));*/
/*    fflush(stdout);*/
/*}*/

int main()
{
    // Get the arbitrary start time of the program to measure difference of time.
    int sched_policy;
    printf("Enter the desired Scheduling technique: \n 1 - First In First Out (Default) \n 2 - Round Robin \n");
    scanf("%d",&sched_policy);
    SCHEDULE_POLICY = sched_policy == 2 ? 2 : 1;
    struct timespec processStartTime;
    clock_gettime(CLOCK_MONOTONIC, &processStartTime);

    // Force the threads to run on one core
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);

    // Declare the 3 required threads
    pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;

    // Create a thread attribute to give to all threads making them run on a single CPU
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);

    // Set the schedule policy for the threads
    pthread_attr_setschedpolicy(&attr, SCHEDULE_POLICY);
    
    // Find the parameters and schedule priority
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHEDULE_POLICY);
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    // Create the first thread and make it run inBetweenChars
    clock_gettime(CLOCK_MONOTONIC, &threadMetric1.releaseTime);
    if (pthread_create(&thread1, &attr, inBetweenChars, "Thread1")) {
        perror("Error creating thread1 \n");
        return 1;
    }

    // Create the second thread and make it run functionPrint
    clock_gettime(CLOCK_MONOTONIC, &threadMetric2.releaseTime);
    if (pthread_create(&thread2, &attr, functionPrint, "Thread2")) {
        perror("Error creating thread2 \n");
        return 1;
    }

    // Create the third thread and make it run sumAvgProduct
    clock_gettime(CLOCK_MONOTONIC, &threadMetric3.releaseTime);
    if (pthread_create(&thread3, &attr, sumAvgProduct, "Thread3")) {
        perror("Error creating thread3 \n");
        return 1;
    }


    // Wait for each of the threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);


    printf("Thread 1 start timestamp: %lfms\n", timespecToMillis(threadMetric1.startTime) - timespecToMillis(processStartTime));
    printf("Thread 1 finish timestamp: %lfms\n", timespecToMillis(threadMetric1.finishTime) - timespecToMillis(processStartTime));
    printf("Thread 1 wait time: %lfms\n", threadMetric1.waitTime);
    printf("Thread 1 execution time: %lfms\n", threadMetric1.executionTime);
    printf("Thread 1 Turnaround Time: %lfms\n", threadMetric1.turnaroundTime);
    printf("Thread 1 Release Time: %lfms\n", threadMetric1.releaseTime);
    printf("Thread 1 CPU Usage: %lf %% \n", threadMetric1.cpuUsage * 100);
    printf("Thread 1 Memory Usage: %ld (KB) \n", threadMetric1.memoryUsage);

    printf("\n######################################\n");

    printf("Thread 2 start timestamp: %lfms\n", timespecToMillis(threadMetric2.startTime) - timespecToMillis(processStartTime));
    printf("Thread 2 finish timestamp: %lfms\n", timespecToMillis(threadMetric2.finishTime) - timespecToMillis(processStartTime));
    printf("Thread 2 wait time: %lfms\n", threadMetric2.waitTime);
    printf("Thread 2 execution time: %lfms\n", threadMetric2.executionTime);
    printf("Thread 2 Turnaround Time: %lfms\n", threadMetric2.turnaroundTime);
    printf("Thread 2 Release Time: %lfms\n", threadMetric2.releaseTime);
    printf("Thread 2 CPU Usage: %lf %% \n", threadMetric2.cpuUsage * 100);
    printf("Thread 2 Memory Usage: %ld (KB) \n", threadMetric2.memoryUsage);

    printf("\n######################################\n");

    printf("Thread 3 start timestamp: %lfms\n", timespecToMillis(threadMetric3.startTime) - timespecToMillis(processStartTime));
    printf("Thread 3 finish timestamp: %lfms\n", timespecToMillis(threadMetric3.finishTime) - timespecToMillis(processStartTime));
    printf("Thread 3 wait time: %lfms\n", threadMetric3.waitTime);
    printf("Thread 3 execution time: %lfms\n", threadMetric3.executionTime);
    printf("Thread 3 Turnaround Time: %lfms\n", threadMetric3.turnaroundTime);
    printf("Thread 3 Release Time: %lfms\n", threadMetric3.releaseTime);
    printf("Thread 3 CPU Usage: %lf %% \n", threadMetric3.cpuUsage * 100);
    printf("Thread 3 Memory Usage: %ld (KB) \n", threadMetric3.memoryUsage);

    return 0;
}
