#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MaxName 32
#define MaxTxPerBlock 4
#define HashHexLen 17
#define QueueCapacity 256
#define MiningDifficulty 2

//transaction struct
typedef struct{
    char sender[MaxName];
    char receiver[MaxName];
    double amount;
    long timestamp;
} Transaction;

//transaction queue
typedef struct{
    Transaction data[QueueCapacity];
    int front;
    int rear;
    int size;
} TransactionQueue;

//block struct
typedef struct{
    int index;
    Transaction transactions[MaxTxPerBlock];
    int tx_count;
    char Previous_hash[HashHexLen];
    char Current_hash[HashHexLen];
    char merkle_root[HashHexLen];
    long timestamp;
    long nonce;
} Block;


//transaction queue functions
//----------------------------------------------
void queue_init(TransactionQueue *q){
    q->front = 0;
    q->rear=-1;
    q->size=0;
}

int queue_is_empty(const TransactionQueue *q){
    if(q->size==0)
        return 1;
    else
        return 0;
}

int queue_enqueue(TransactionQueue *q, Transaction tx){
    if(q->size>=QueueCapacity)
        return 0;
    q->rear=(q->rear+1)%QueueCapacity;
    q->data[q->rear]=tx;
    q->size++;
    return 1;
}

Transaction queue_dequeue(TransactionQueue *q){
    Transaction tx=q->data[q->front];
    q->front=(q->front+1)%QueueCapacity;
    q->size--;
    return tx;
}
//------------------------------------------------------

//hashing algorithm
unsigned long djb2_hash(const char *str){
    unsigned long hash=5381;
    int c;
    while((c=*str++))
        hash=((hash<<5)+hash)+(unsigned long)c;
    return hash;
}

void hash_to_hex(unsigned long h, char *out){
    sprintf(out, "%016lx", h);
}

//upside down binary tree - merkle tree
void hash_treatment(const Transaction *tx, char *out){
    char buf[256];
    snprintf("");
}

