#define _GNU_SOURCE
#include "ThreadMetric.h"
#include "TimeUtils.h"
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <malloc.h>


// Structs to store details about each thread's performance metrics
// These are global so that they can be accessed from any method
ThreadMetric threadMetric1;
ThreadMetric threadMetric2;
ThreadMetric threadMetric3;

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
    int product = 1;
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
    printf("\nThe product is : %d", product);
    printf("\nThe average is : %.2f\n", average);

    clock_gettime(CLOCK_MONOTONIC, &threadMetric3.finishTime);
    threadMetric3.waitTime = timespecToMillis(totalWaitTime);
    subtractTimespec(&temp, &threadMetric3.finishTime, &threadMetric3.releaseTime);
    threadMetric3.turnaroundTime = timespecToMillis(temp);
    threadMetric3.executionTime = threadMetric3.turnaroundTime - threadMetric3.waitTime;
    threadMetric3.responseTime = timespecToMillis(threadMetric3.startTime) - timespecToMillis(threadMetric3.releaseTime);
    threadMetric3.cpuUsage = threadMetric3.executionTime / (threadMetric3.executionTime + threadMetric3.waitTime);


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
    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setaffinity_np(&threadAttr, sizeof(cpu_set_t), &cpuset);

    // Create the first thread and make it run inBetweenChars
    clock_gettime(CLOCK_MONOTONIC, &threadMetric1.releaseTime);
    pthread_create(&thread1, &threadAttr, inBetweenChars, NULL);

    // Create the second thread and make it run functionPrint
    clock_gettime(CLOCK_MONOTONIC, &threadMetric2.releaseTime);
    pthread_create(&thread2, &threadAttr, functionPrint, NULL);

    // Create the third thread and make it run sumAvgProduct
    clock_gettime(CLOCK_MONOTONIC, &threadMetric3.releaseTime);
    pthread_create(&thread3, &threadAttr, sumAvgProduct, NULL);

    struct sched_param param;
    param.sched_priority = 50;
    pthread_setschedparam(thread1, SCHED_FIFO, &param);
    pthread_setschedparam(thread2, SCHED_FIFO, &param);
    pthread_setschedparam(thread3, SCHED_FIFO, &param);

    // Wait for each of the threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);


    printf("Thread 1 start timestamp: %lfms\n", timespecToMillis(threadMetric1.startTime) - timespecToMillis(processStartTime));
    printf("Thread 1 finish timestamp: %lfms\n", timespecToMillis(threadMetric1.finishTime) - timespecToMillis(processStartTime));
    printf("Thread 1 wait time: %lfms\n", threadMetric1.waitTime);
    printf("Thread 1 execution time: %lfms\n", threadMetric1.executionTime);
    printf("Thread 1 Turnaround Time: %lfms\n", threadMetric1.turnaroundTime);
    printf("Thread 1 CPU Usage: %lf %% \n", threadMetric1.cpuUsage * 100);

    printf("\n######################################\n");

    printf("Thread 2 start timestamp: %lfms\n", timespecToMillis(threadMetric2.startTime) - timespecToMillis(processStartTime));
    printf("Thread 2 finish timestamp: %lfms\n", timespecToMillis(threadMetric2.finishTime) - timespecToMillis(processStartTime));
    printf("Thread 2 wait time: %lfms\n", threadMetric2.waitTime);
    printf("Thread 2 execution time: %lfms\n", threadMetric2.executionTime);
    printf("Thread 2 Turnaround Time: %lfms\n", threadMetric2.turnaroundTime);
    printf("Thread 2 CPU Usage: %lf %% \n", threadMetric2.cpuUsage * 100);

    printf("\n######################################\n");

    printf("Thread 3 start timestamp: %lfms\n", timespecToMillis(threadMetric3.startTime) - timespecToMillis(processStartTime));
    printf("Thread 3 finish timestamp: %lfms\n", timespecToMillis(threadMetric3.finishTime) - timespecToMillis(processStartTime));
    printf("Thread 3 wait time: %lfms\n", threadMetric3.waitTime);
    printf("Thread 3 execution time: %lfms\n", threadMetric3.executionTime);
    printf("Thread 3 Turnaround Time: %lfms\n", threadMetric3.turnaroundTime);
    printf("Thread 3 CPU Usage: %lf %% \n", threadMetric3.cpuUsage * 100);

    return 0;
}
