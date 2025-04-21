#pragma once

#include <pthread.h>
#include <time.h>

void safeMutexLock(pthread_mutex_t *mutex, struct timespec *waitAccumulator);