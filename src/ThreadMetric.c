#include "ThreadMetric.h"
#include <time.h>

void initializeThreadMetric(ThreadMetric *metric, pthread_t threadId)
{

    // initializing all struct fields with 0 at first
    metric->executionTime = 0;
    metric->memoryUsage = 0;
    metric->responseTime = 0;
    metric->threadId = threadId;
    metric->turnaroundTime = 0;
    metric->waitTime = 0;

    // initialize start and finish time with values;
    clock_gettime(CLOCK_MONOTONIC, &metric->releaseTime);
    clock_gettime(CLOCK_MONOTONIC, &metric->finishTime);
}

double timespecToMillis(struct timespec ts)
{
    long seconds = ts.tv_sec;
    long nanos = ts.tv_nsec;

    return (double)seconds * 1000 + nanos / 10e6;
}