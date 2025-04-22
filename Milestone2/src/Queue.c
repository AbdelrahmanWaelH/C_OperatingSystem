#include <stdbool.h>
#include <stdio.h>
#include "../include/Queue.h"


void Q_init(Queue* q){

    q->count = 0;
    for(int i = 0; i < 32; i++){
        q->arr[i] = 0;
    }

}

void Q_enqueue(Queue* q, int val){
    q->arr[q->count++] = val;
}

int Q_dequeue(Queue* q){

    int retVal = q->arr[q->count];

    for(int i = 1; i < q->count; i++){
        q->arr[i-1] = q->arr[i];
    }

    q->count--;
    return retVal;

}

bool Q_isFull(Queue* q){
    return q->count == 32;
}

bool Q_isEmpty(Queue* q){
    return q->count == 0;
}

int Q_getFirst(Queue* q){
    return q->arr[0];
}

void Q_display(Queue* q){

    for(int i = 0; i < q->count-1; i++){
        printf("%d, ", q->arr[i]);
    }
    printf("%d\n", q->arr[q->count-1]);


}

