#include "types.h"
#include "queue.h"

void queue_init(TransactionQueue *q){
    q->front = 0;
    q->rear = -1;
    q->size = 0;
}

int queue_is_empty(const TransactionQueue *q){
    if(q->size == 0)
        return 1;
    else
        return 0;
}

int queue_enqueue(TransactionQueue *q, Transaction tx){
    if(q->size >= QueueCapacity){
        return 0;
    }
    q->rear = (q->rear + 1) % QueueCapacity;
    q->data[q->rear] = tx;
    q->size = q->size + 1;
    return 1;
}

Transaction queue_dequeue(TransactionQueue *q){
    Transaction tx = q->data[q->front];
    q->front = (q->front + 1) % QueueCapacity;
    q->size = q->size - 1;
    return tx;
}
