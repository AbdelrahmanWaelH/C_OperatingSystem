#include<stdio.h>
#include <stdlib.h>
#include "../include/Queue.h"

Queue *queue_new() {
    Queue *q = (Queue *)malloc(sizeof(Queue));
    q->head = q->tail = NULL;
    q->length = 0;
    return q;
}

void queue_push_tail(Queue *q, void *data) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->data = data;
    n->next = NULL;
    if (!q->tail) q->head = n;
    else q->tail->next = n;
    q->tail = n;
    q->length++;
}

void *queue_pop_head(Queue *q) {
    if (!q->head) return NULL;
    Node *n = q->head;
    void *data = n->data;
    q->head = n->next;
    if (!q->head) q->tail = NULL;
    free(n);
    q->length--;
    return data;
}

int queue_is_empty(Queue *q) { return q->length == 0; }
int queue_length(Queue *q) { return q->length; }

void queue_free(Queue *q) {
    while (!queue_is_empty(q)) queue_pop_head(q);
    free(q);
}