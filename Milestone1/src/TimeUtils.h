#pragma once
#include <time.h>

void addTimespec(struct timespec *result, struct timespec *t1, struct timespec *t2);
void subtractTimespec(struct timespec *result, struct timespec *t1, struct timespec *t2);