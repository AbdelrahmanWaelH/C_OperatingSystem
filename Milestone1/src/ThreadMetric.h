#pragma once
#include <time.h>
#include <pthread.h>

typedef struct
{
    pthread_t threadId;
    struct timespec releaseTime;
    struct timespec startTime;
    struct timespec finishTime;
    double executionTime;
    double waitTime;
    double responseTime;
    double turnaroundTime;
    double cpuUsage;
    long memoryUsage;

} ThreadMetric;

double timespecToMillis(struct timespec ts);