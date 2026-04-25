#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "types.h"
#include "hash.h"
#include "merkle.h"
#include "blockchain.h"

void compute_block_hash(const Block *b, char *out){
    char buffer[1024];
    int pos = 0;
    int j;

    long idx = (long)b->index;
    char idx_str[32];
    int idx_pos = 0;
    if(idx == 0){
        idx_str[0] = '0';
        idx_pos = 1;
    }
    else{
        long temp = idx;
        char temp_buf[32];
        int tbp = 0;
        while(temp > 0){
            temp_buf[tbp] = (temp % 10) + '0';
            tbp = tbp + 1;
            temp = temp / 10;
        }
        int k;
        for(k = 0; k < tbp; k++){
            idx_str[idx_pos] = temp_buf[tbp - 1 - k];
            idx_pos = idx_pos + 1;
        }
    }
    idx_str[idx_pos] = '\0';
    for(j = 0; idx_str[j] != '\0' && pos < 1000; j++){
        buffer[pos] = idx_str[j];
        pos = pos + 1;
    }

    j = 0;
    while(b->Previous_hash[j] != '\0' && pos < 1000){
        buffer[pos] = b->Previous_hash[j];
        pos = pos + 1;
        j = j + 1;
    }

    j = 0;
    while(b->merkle_root[j] != '\0' && pos < 1000){
        buffer[pos] = b->merkle_root[j];
        pos = pos + 1;
        j = j + 1;
    }

    long ts = b->timestamp;
    char ts_str[32];
    int ts_pos = 0;
    if(ts == 0){
        ts_str[0] = '0';
        ts_pos = 1;
    }
    else{
        long temp = ts;
        char tbuf[32];
        int tdp = 0;
        while(temp > 0){
            tbuf[tdp] = (temp % 10) + '0';
            tdp = tdp + 1;
            temp = temp / 10;
        }
        int k;
        for(k = 0; k < tdp; k++){
            ts_str[ts_pos] = tbuf[tdp - 1 - k];
            ts_pos = ts_pos + 1;
        }
    }
    ts_str[ts_pos] = '\0';
    for(j = 0; ts_str[j] != '\0' && pos < 1000; j++){
        buffer[pos] = ts_str[j];
        pos = pos + 1;
    }

    long nc = (long)b->nonce;
    char nc_str[32];
    int nc_pos = 0;
    if(nc == 0){
        nc_str[0] = '0';
        nc_pos = 1;
    }
    else{
        long temp = nc;
        char nbuf[32];
        int nbp = 0;
        while(temp > 0){
            nbuf[nbp] = (temp % 10) + '0';
            nbp = nbp + 1;
            temp = temp / 10;
        }
        int k;
        for(k = 0; k < nbp; k++){
            nc_str[nc_pos] = nbuf[nbp - 1 - k];
            nc_pos = nc_pos + 1;
        }
    }
    nc_str[nc_pos] = '\0';
    for(j = 0; nc_str[j] != '\0' && pos < 1000; j++){
        buffer[pos] = nc_str[j];
        pos = pos + 1;
    }

    buffer[pos] = '\0';

    unsigned long h = djb2_hash(buffer);
    hash_to_hex(h, out);
}

void mine_block(Block *b){
    b->nonce = 0;
    char hash_out[HashHexLen];

    while(1){
        b->nonce = b->nonce + 1;
        compute_block_hash(b, hash_out);

        int all_zeros = 1;
        int i;
        for(i = 0; i < MiningDifficulty; i++){
            if(hash_out[i] != '0'){
                all_zeros = 0;
                break;
            }
        }

        if(all_zeros == 1){
            break;
        }
    }

    int k;
    for(k = 0; k < 16; k++){
        b->Current_hash[k] = hash_out[k];
    }
    b->Current_hash[16] = '\0';

    printf("Block %d mined!  nonce=%lu  hash=%.12s...\n",
           b->index, b->nonce, b->Current_hash);
}

void blockchain_init(Blockchain *bc){
    BlockNode *genesis = (BlockNode *)malloc(sizeof(BlockNode));
    Block *b = &genesis->block;

    b->index = 0;
    b->tx_count = 1;

    int i = 0;
    while("SYSTEM"[i] != '\0' && i < MaxName - 1){
        b->transactions[0].sender[i] = "SYSTEM"[i];
        i = i + 1;
    }
    b->transactions[0].sender[i] = '\0';

    i = 0;
    while("Genesis"[i] != '\0' && i < MaxName - 1){
        b->transactions[0].receiver[i] = "Genesis"[i];
        i = i + 1;
    }
    b->transactions[0].receiver[i] = '\0';

    b->transactions[0].amount = 0.0;
    b->transactions[0].timestamp = time(NULL);

    int k;
    for(k = 0; k < 16; k++){
        b->Previous_hash[k] = '0';
    }
    b->Previous_hash[16] = '\0';

    b->timestamp = time(NULL);

    compute_merkle_root(b->transactions, b->tx_count, b->merkle_root);
    mine_block(b);

    genesis->next = NULL;
    bc->head = genesis;
    bc->tail = genesis;
    bc->length = 1;

    printf("Genesis block created!\n");
}

void blockchain_add_block(Blockchain *bc, Transaction txns[], int tx_count){
    BlockNode *node = (BlockNode *)malloc(sizeof(BlockNode));
    Block *b = &node->block;

    b->index = bc->length;
    b->tx_count = tx_count;
    b->timestamp = time(NULL);

    int k;
    for(k = 0; k < tx_count; k++){
        b->transactions[k] = txns[k];
    }

    int h;
    for(h = 0; h < 16; h++){
        b->Previous_hash[h] = bc->tail->block.Current_hash[h];
    }
    b->Previous_hash[16] = '\0';

    compute_merkle_root(b->transactions, b->tx_count, b->merkle_root);

    mine_block(b);

    node->next = NULL;
    bc->tail->next = node;
    bc->tail = node;
    bc->length = bc->length + 1;
}

int blockchain_verify(const Blockchain *bc){
    BlockNode *prev = bc->head;
    BlockNode *curr = bc->head->next;
    int valid = 1;

    while(curr != NULL){
        Block *b = &curr->block;

        char expected_hash[HashHexLen];
        compute_block_hash(b, expected_hash);
        if(strcmp(b->Current_hash, expected_hash) != 0){
            printf("TAMPER @ Block %d: hash mismatch!\n", b->index);
            printf("  stored : %s\n", b->Current_hash);
            printf("  actual : %s\n", expected_hash);
            valid = 0;
        }

        if(strcmp(b->Previous_hash, prev->block.Current_hash) != 0){
            printf("TAMPER @ Block %d: chain link broken!\n", b->index);
            valid = 0;
        }

        char expected_mr[HashHexLen];
        compute_merkle_root(b->transactions, b->tx_count, expected_mr);
        if(strcmp(b->merkle_root, expected_mr) != 0){
            printf("TAMPER @ Block %d: merkle root mismatch!\n", b->index);
            valid = 0;
        }

        prev = curr;
        curr = curr->next;
    }

    if(valid)
        printf("Blockchain VALID (%d blocks verified).\n", bc->length);
    return valid;
}

void blockchain_tamper(Blockchain *bc, int block_idx, const char *new_receiver, double new_amount){
    BlockNode *cur = bc->head;
    int i = 0;
    while(i < block_idx && cur != NULL){
        cur = cur->next;
        i = i + 1;
    }

    if(cur == NULL || cur->block.tx_count == 0){
        printf("Invalid block index.\n");
        return;
    }

    printf("\n--- TAMPERING Block %d ---\n", block_idx);
    printf("  Before: %s -> %s : %.2f\n",
           cur->block.transactions[0].sender,
           cur->block.transactions[0].receiver,
           cur->block.transactions[0].amount);

    int j = 0;
    while(new_receiver[j] != '\0' && j < MaxName - 1){
        cur->block.transactions[0].receiver[j] = new_receiver[j];
        j = j + 1;
    }
    cur->block.transactions[0].receiver[j] = '\0';
    cur->block.transactions[0].amount = new_amount;

    printf("  After : %s -> %s : %.2f\n",
           cur->block.transactions[0].sender,
           new_receiver, new_amount);
    printf("--- Chain verification will now FAIL. ---\n");
}

void blockchain_print_ledger(const Blockchain *bc){
    printf("\n");
    printf("==========================================\n");
    printf("          BLOCKCHAIN LEDGER               \n");
    printf("==========================================\n");

    BlockNode *cur = bc->head;
    while(cur != NULL){
        Block *b = &cur->block;
        printf("+--------------------------------------+\n");
        printf("| Block #%d\n", b->index);
        printf("| Timestamp  : %ld\n", b->timestamp);
        printf("| Prev Hash  : %.12s...\n", b->Previous_hash);
        printf("| Hash       : %.12s...\n", b->Current_hash);
        printf("| Merkle Root: %.12s...\n", b->merkle_root);
        printf("| Nonce      : %lu\n", b->nonce);
        printf("| Transactions:\n");
        int i;
        for(i = 0; i < b->tx_count; i++){
            printf("|   %s -> %s : %.2f coins\n",
                   b->transactions[i].sender,
                   b->transactions[i].receiver,
                   b->transactions[i].amount);
        }
        printf("+--------------------------------------+\n");
        if(cur->next != NULL)
            printf("          |\n          v\n");
        cur = cur->next;
    }
}

void blockchain_free(Blockchain *bc){
    BlockNode *cur = bc->head;
    while(cur != NULL){
        BlockNode *tmp = cur;
        cur = cur->next;
        free(tmp);
    }
    bc->head = NULL;
    bc->tail = NULL;
    bc->length = 0;
}
