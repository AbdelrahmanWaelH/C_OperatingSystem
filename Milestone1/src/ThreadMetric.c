#include "ThreadMetric.h"
#include <time.h>

double timespecToMillis(struct timespec ts)
{
    long seconds = ts.tv_sec;
    long nanos = ts.tv_nsec;

    return (double)seconds * 1000 + nanos / 10e6;
}