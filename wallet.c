#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "wallet.h"

void wallet_init(WalletList *wl){
    wl->head = NULL;
    wl->count = 0;
}

WalletNode *wallet_find(WalletList *wl, const char *name){
    WalletNode *cur = wl->head;
    while(cur != NULL){
        if(strcmp(cur->name, name) == 0)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

void wallet_update(WalletList *wl, const char *name, double delta){
    WalletNode *node = wallet_find(wl, name);
    if(node != NULL){
        node->balance = node->balance + delta;
    }
    else{
        WalletNode *new_node = (WalletNode *)malloc(sizeof(WalletNode));
        int i = 0;
        while(name[i] != '\0' && i < MaxName - 1){
            new_node->name[i] = name[i];
            i = i + 1;
        }
        new_node->name[i] = '\0';
        new_node->balance = delta;
        new_node->next = wl->head;
        wl->head = new_node;
        wl->count = wl->count + 1;
    }
}

void wallet_print_all(const WalletList *wl){
    printf("\n===== WALLET BALANCES =====\n");
    WalletNode *cur = wl->head;
    while(cur != NULL){
        printf("  %-12s : %.2f coins\n", cur->name, cur->balance);
        cur = cur->next;
    }
    printf("===========================\n");
}

void wallet_free(WalletList *wl){
    WalletNode *cur = wl->head;
    while(cur != NULL){
        WalletNode *tmp = cur;
        cur = cur->next;
        free(tmp);
    }
    wl->head = NULL;
    wl->count = 0;
}
