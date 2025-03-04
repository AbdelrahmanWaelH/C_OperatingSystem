#define _GNU_SOURCE
#include "ThreadMetric.h"
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <malloc.h>

// Create a mutex that handles the sharing of the stdout/stdin resource
// All uses of stdout/stdin (printf and scanf) will be wrapped in locking/unlocking of mutex to handle sharing resources correctly
pthread_mutex_t stdoutMutex = PTHREAD_MUTEX_INITIALIZER;

// Structs to store details about each thread's performance metrics
// These are global so that they can be accessed from any method
ThreadMetric threadMetric1;
ThreadMetric threadMetric2;
ThreadMetric threadMetric3;

void *inBetweenChars(void *arg)
{ // Thread 1 start

    // Record thread start time.
    clock_gettime(CLOCK_MONOTONIC, &threadMetric1.startTime);

    char start, end;

    pthread_mutex_lock(&stdoutMutex);
    printf("Enter the start character: \n");
    scanf(" %c", &start);
    pthread_mutex_unlock(&stdoutMutex);

    pthread_mutex_lock(&stdoutMutex);
    printf("Enter the end character: \n");
    scanf(" %c", &end);
    pthread_mutex_unlock(&stdoutMutex);

    /*if(start > end) swap(start, end)*/

    while (start <= end)
    {
        pthread_mutex_lock(&stdoutMutex);
        printf("%c\n", start);
        pthread_mutex_unlock(&stdoutMutex);
        start++;
    }

    clock_gettime(CLOCK_MONOTONIC, &threadMetric1.finishTime);

} // Thread 1 end

void *sumAvgProduct(void *arg)
{ // Thread 3 start
    int start, end;
    int sum = 0;
    int product = 1;
    float average = 0;

    pthread_mutex_lock(&stdoutMutex);
    printf("Enter the start integer: \n");
    scanf(" %d", &start);
    pthread_mutex_unlock(&stdoutMutex);

    pthread_mutex_lock(&stdoutMutex);
    printf("Enter the end integer: \n");
    scanf(" %d", &end);
    pthread_mutex_unlock(&stdoutMutex);

    /*if(start > end) swap(start, end)*/

    int delta = end - start + 1;

    while (start <= end)
    {
        sum += start;
        product *= start;
        start++;
    }

    average = (float)sum / delta;

    pthread_mutex_lock(&stdoutMutex);
    printf("\nThe sum is : %d", sum);
    pthread_mutex_unlock(&stdoutMutex);

    pthread_mutex_lock(&stdoutMutex);
    printf("\nThe product is : %d", product);
    pthread_mutex_unlock(&stdoutMutex);

    pthread_mutex_lock(&stdoutMutex);
    printf("\nThe average is : %.2f\n", average);
    pthread_mutex_unlock(&stdoutMutex);

} // Thread 3 end

void *functionPrint(void *arg)
{
    pthread_t threadId = pthread_self();

    pthread_mutex_lock(&stdoutMutex);
    printf("Welcome to the printing park!\n");
    pthread_mutex_unlock(&stdoutMutex);

    pthread_mutex_lock(&stdoutMutex);
    printf("Brought to you by thread with id : %lu \n", threadId);
    pthread_mutex_unlock(&stdoutMutex);

    pthread_mutex_lock(&stdoutMutex);
    printf("Enjoy your stay!\n");
    pthread_mutex_unlock(&stdoutMutex);
}

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

    // Create the first thread and make it run inBetweenChars
    initializeThreadMetric(&threadMetric1, thread1);
    clock_gettime(CLOCK_MONOTONIC, &threadMetric1.releaseTime);
    pthread_create(&thread1, NULL, inBetweenChars, NULL);

    // Create the second thread and make it run functionPrint
    pthread_create(&thread2, NULL, functionPrint, NULL);

    // Create the second thread and make it run sumAvgProduct
    pthread_create(&thread3, NULL, sumAvgProduct, NULL);

    // Wait for each of the threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    printf("Thread 1 start: %lfms\n", timespecToMillis(threadMetric1.startTime) - timespecToMillis(processStartTime));

    printf("Thread 1 finish: %lfms\n", timespecToMillis(threadMetric1.finishTime) - timespecToMillis(processStartTime));

    printf("Thread 1 Total Execution Time: %lfms\n", timespecToMillis(threadMetric1.finishTime) - timespecToMillis(threadMetric1.startTime));

    return 0;
}