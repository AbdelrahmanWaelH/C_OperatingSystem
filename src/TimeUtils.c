#include "TimeUtils.h"

void addTimespec(struct timespec *result, struct timespec *t1, struct timespec *t2)
{
    result->tv_sec = t1->tv_sec + t2->tv_sec;
    result->tv_nsec = t1->tv_nsec + t2->tv_nsec;

    if (result->tv_nsec > 10e9)
    {
        result->tv_sec += 1;
        result->tv_nsec -= 10e9;
    }
}

void subtractTimespec(struct timespec *result, struct timespec *t1, struct timespec *t2)
{
    result->tv_sec = t1->tv_sec - t2->tv_sec;
    result->tv_nsec = t1->tv_nsec - t2->tv_nsec;

    if (result->tv_nsec < 0)
    {
        result->tv_sec -= 1;
        result->tv_nsec += 10e9;
    }
}