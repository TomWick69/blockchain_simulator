#include <stdio.h>
#include <string.h>
#include "types.h"
#include "search.h"

void search_transactions_by_user(const Blockchain *bc, const char *name){
    printf("\n===== TRANSACTIONS involving '%s' =====\n", name);
    int found = 0;
    BlockNode *cur = bc->head;

    while(cur != NULL){
        Block *b = &cur->block;
        int i;
        for(i = 0; i < b->tx_count; i++){
            int is_sender = (strcmp(b->transactions[i].sender, name) == 0);
            int is_receiver = (strcmp(b->transactions[i].receiver, name) == 0);

            if(is_sender == 1 || is_receiver == 1){
                printf("  Block #%d: %s -> %s : %.2f coins\n",
                       b->index,
                       b->transactions[i].sender,
                       b->transactions[i].receiver,
                       b->transactions[i].amount);
                found = found + 1;
            }
        }
        cur = cur->next;
    }

    if(found == 0){
        printf("  No transactions found.\n");
    }
    printf("  Total: %d transaction(s)\n", found);
    printf("==========================================\n");
}

Block *find_block_by_index(const Blockchain *bc, int index){
    BlockNode *cur = bc->head;

    while(cur != NULL){
        if(cur->block.index == index){
            return &cur->block;
        }
        cur = cur->next;
    }

    return NULL;
}

Block *find_block_by_hash(const Blockchain *bc, const char *hash_prefix){
    BlockNode *cur = bc->head;
    int prefix_len = 0;

    while(hash_prefix[prefix_len] != '\0'){
        prefix_len = prefix_len + 1;
    }

    while(cur != NULL){
        int match = 1;
        int i;
        for(i = 0; i < prefix_len; i++){
            if(cur->block.Current_hash[i] != hash_prefix[i]){
                match = 0;
                break;
            }
        }

        if(match == 1){
            return &cur->block;
        }
        cur = cur->next;
    }

    return NULL;
}
