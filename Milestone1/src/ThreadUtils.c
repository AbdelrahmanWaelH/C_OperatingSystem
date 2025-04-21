#define _GNU_SOURCE
#include "ThreadUtils.h"
#include <time.h>
#include "TimeUtils.h"

void safeMutexLock(pthread_mutex_t *mutex, struct timespec *waitAccumulator)
{

    struct timespec lockAttemptTime, lockAcquiredTime, diff;

    clock_gettime(CLOCK_MONOTONIC, &lockAttemptTime);
    pthread_mutex_lock(mutex);
    clock_gettime(CLOCK_MONOTONIC, &lockAcquiredTime);
    subtractTimespec(&diff, &lockAcquiredTime, &lockAttemptTime);
    addTimespec(waitAccumulator, waitAccumulator, &diff);
}
