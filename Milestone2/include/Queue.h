#include <stdbool.h>
#include <stdio.h>

typedef struct {

    int arr[32];
    int count;

} Queue;

void Q_init(Queue* q);
void Q_enqueue(Queue* q, int val);
int Q_dequeue(Queue* q);
bool Q_isFull(Queue* q);
bool Q_isEmpty(Queue* q);
int Q_getFirst(Queue* q);

