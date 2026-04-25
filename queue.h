#ifndef QUEUE_H
#define QUEUE_H

#include "types.h"

void queue_init(TransactionQueue *q);
int queue_is_empty(const TransactionQueue *q);
int queue_enqueue(TransactionQueue *q, Transaction tx);
Transaction queue_dequeue(TransactionQueue *q);

#endif
