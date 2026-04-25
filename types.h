#ifndef TYPES_H
#define TYPES_H

#define MaxName 32
#define MaxTxPerBlock 4
#define HashHexLen 17
#define QueueCapacity 256
#define MiningDifficulty 2

typedef struct{
    char sender[MaxName];
    char receiver[MaxName];
    double amount;
    long timestamp;
} Transaction;

typedef struct{
    Transaction data[QueueCapacity];
    int front;
    int rear;
    int size;
} TransactionQueue;

typedef struct{
    int index;
    Transaction transactions[MaxTxPerBlock];
    int tx_count;
    char Previous_hash[HashHexLen];
    char Current_hash[HashHexLen];
    char merkle_root[HashHexLen];
    long timestamp;
    unsigned long nonce;
} Block;

typedef struct BlockNode{
    Block block;
    struct BlockNode *next;
} BlockNode;

typedef struct{
    BlockNode *head;
    BlockNode *tail;
    int length;
} Blockchain;

typedef struct WalletNode{
    char name[MaxName];
    double balance;
    struct WalletNode *next;
} WalletNode;

typedef struct{
    WalletNode *head;
    int count;
} WalletList;

#endif
