#define _GNU_SOURCE
#include "ThreadMetric.h"
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <malloc.h>

// Create a mutex that handles the sharing of the stdout/stdin resource
// All uses of stdout/stdin (printf and scanf) will be wrapped in locking/unlocking of mutex to handle sharing resources correctly
pthread_mutex_t stdoutMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t errorMutex = PTHREAD_MUTEX_INITIALIZER;

// Structs to store details about each thread's performance metrics
// These are global so that they can be accessed from any method
ThreadMetric threadMetric1;
ThreadMetric threadMetric2;
ThreadMetric threadMetric3;

// Error logging function with thread-safe logging
void logError(const char* message) {
    pthread_mutex_lock(&errorMutex);
    fprintf(stderr, "ERROR: %s (errno: %d, %s)\n", 
            message, errno, strerror(errno));
    pthread_mutex_unlock(&errorMutex);
}

int safeInput(const char* prompt, const char* format, void* value) {
    int scanResult;
    do{
        pthread_mutex_lock(&stdoutMutex);
        printf("%s", prompt);
        scanResult = scanf(format, value);
        if (scanResult != 1) {
            printf("Invalid input.\n");
        }    
        pthread_mutex_unlock(&stdoutMutex);
    }while(scanResult == 1);

    return 0;
}

void createThreadWithErrorHandling(pthread_t *thread, void *(*start_routine)(void *), ThreadMetric *metric) {
    initializeThreadMetric(metric, *thread);
    clock_gettime(CLOCK_MONOTONIC, &metric->releaseTime);
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    pthread_setaffinity_np(thread, sizeof(cpuset), &cpuset);
    int createResult = pthread_create(thread, NULL, start_routine, NULL);
    if (createResult != 0) {
        char errorMsg[256];
        snprintf(errorMsg, sizeof(errorMsg), 
                 "Failed to create thread (error code: %d)", createResult);
        logError(errorMsg);
        exit(1);
    }
}

void *inBetweenChars(void *arg)
{
    struct timespec inputWaitStart = {0};
    struct timespec inputWaitEnd = {0};
    struct timespec timeDifference = {0};
    struct timespec totalWaitTime = {0};
    struct timespec temp = {0};

    // Record thread start time
    clock_gettime(CLOCK_MONOTONIC, &threadMetric1.startTime);

    char start, end;

    // Get start character
    clock_gettime(CLOCK_MONOTONIC, &inputWaitStart);
    if (safeInput("Enter the start character: ", "%c", &start) != 0) {
        return NULL;
    }
    clock_gettime(CLOCK_MONOTONIC, &inputWaitEnd);

    subtractTimespec(&timeDifference, &inputWaitEnd, &inputWaitStart);
    addTimespec(&totalWaitTime, &totalWaitTime, &timeDifference);

    // Get end character
    clock_gettime(CLOCK_MONOTONIC, &inputWaitStart);
    if (safeInput("Enter the end character: ", "%c", &end) != 0) {
        return NULL;
    }
    clock_gettime(CLOCK_MONOTONIC, &inputWaitEnd);

    subtractTimespec(&timeDifference, &inputWaitEnd, &inputWaitStart);
    addTimespec(&totalWaitTime, &totalWaitTime, &timeDifference);

    // Ensure start is less than or equal to end
    if (start > end) {
        char temp = start;
        start = end;
        end = temp;
    }

    // Print characters from start to end
    while (start <= end)
    {
        pthread_mutex_lock(&stdoutMutex);
        printf("%c\n", start);
        pthread_mutex_unlock(&stdoutMutex);
        start++;
    }

    clock_gettime(CLOCK_MONOTONIC, &threadMetric1.finishTime);

    // Compute thread metrics
    threadMetric1.waitTime = timespecToMillis(totalWaitTime);
    subtractTimespec(&temp, &threadMetric1.finishTime, &threadMetric1.releaseTime);
    threadMetric1.turnaroundTime = timespecToMillis(temp);
    threadMetric1.executionTime = threadMetric1.turnaroundTime - threadMetric1.waitTime;

    return NULL;
}

void *sumAvgProduct(void *arg)
{
    int start, end;
    int sum = 0;
    long long product = 1;
    float average = 0;

    // Use safe input functions
    if (safeInput("Enter the start integer: \n", '%d' ,&start) != 0) {
        return NULL;
    }

    if (safeInput("Enter the end integer: \n",'%d', &end) != 0) {
        return NULL;
    }

    // Ensure start is less than or equal to end
    if (start > end) {
        int temp = start;
        start = end;
        end = temp;
    }    

    int delta = end - start + 1;

    while (start <= end)
    {
        // Protect against integer overflow
        if (sum > INT_MAX - start || product > LLONG_MAX / start) {
            logError("Arithmetic overflow detected");
            break;
        }
        sum += start;
        product *= start;
        start++;
    }

    average = (float)sum / delta;

    pthread_mutex_lock(&stdoutMutex);
    printf("\nResults:\n---------\n");
    printf("Sum: %d\n", sum);
    printf("Product: %lld\n", product);
    printf("Average: %.2f\n", average);
    pthread_mutex_unlock(&stdoutMutex);

    return NULL;
}

void *functionPrint(void *arg)
{
    pthread_t threadId = pthread_self();

    pthread_mutex_lock(&stdoutMutex);
    printf("##############\nWelcome to the printing park!\n");
    printf("Brought to you by thread with id : %lu \n", threadId);
    printf("Enjoy your stay!\n#################\n");
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
    pthread_t thread1, thread2, thread3;

    // Create threads with improved error handling
    createThreadWithErrorHandling(&thread1, inBetweenChars, &threadMetric1);
    createThreadWithErrorHandling(&thread2, functionPrint, &threadMetric2);
    createThreadWithErrorHandling(&thread3, sumAvgProduct, &threadMetric3);

    // Wait for each of the threads to finish with error handling
    int joinResult1 = pthread_join(thread1, NULL);
    int joinResult2 = pthread_join(thread2, NULL);
    int joinResult3 = pthread_join(thread3, NULL);
    
    // Check for join errors
    if (joinResult1 != 0 || joinResult2 != 0 || joinResult3 != 0) {
        logError("One or more thread joins failed");
    }

    printf("Thread 1 start timestamp: %lfms\n", timespecToMillis(threadMetric1.startTime) - timespecToMillis(processStartTime));

    printf("Thread 1 finish timestamp: %lfms\n", timespecToMillis(threadMetric1.finishTime) - timespecToMillis(processStartTime));

    printf("Thread 1 wait time: %lfms\n", threadMetric1.waitTime);

    printf("Thread 1 execution time: %lfms\n", threadMetric1.executionTime);

    printf("Thread 1 Turnaround Time: %lfms\n", threadMetric1.turnaroundTime);

    return 0;
}