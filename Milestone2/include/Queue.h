#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>

typedef struct Node {
    void *data;
    struct Node *next;
} Node;

typedef struct Queue {
    Node *head;
    Node *tail;
    int length;
} Queue;

Queue *queue_new();
void queue_push_tail(Queue *q, void *data);
void *queue_pop_head(Queue *q);
int queue_is_empty(Queue *q);
int queue_length(Queue *q);
void queue_free(Queue *q);

#endif
