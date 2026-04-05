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
    unsigned long nonce;
} Block;

//blockchain linked list node
typedef struct BlockNode{
    Block block;
    struct BlockNode *next;
} BlockNode;

//blockchain struct
typedef struct{
    BlockNode *head;
    BlockNode *tail;
    int length;
} Blockchain;

//wallet linked list node
typedef struct WalletNode{
    char name[MaxName];
    double balance;
    struct WalletNode *next;
} WalletNode;

//wallet list
typedef struct{
    WalletNode *head;
    int count;
} WalletList;


//transaction queue functions
//----------------------------------------------
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
//------------------------------------------------------


//hashing algorithm
unsigned long djb2_hash(const char *str){
    unsigned long hash = 5381;
    int i = 0;
    int c;
    while((c = str[i]) != '\0'){
        hash = ((hash << 5) + hash) + (unsigned long)c;
        i = i + 1;
    }
    return hash;
}

void hash_to_hex(unsigned long h, char *out){
    char hex_chars[] = "0123456789abcdef";
    char temp[20];
    int pos = 0;
    int i;

    if(h == 0){
        strcpy(out, "0000000000000000");
        return;
    }

    while(h > 0){
        int remainder = h % 16;
        temp[pos] = hex_chars[remainder];
        pos = pos + 1;
        h = h / 16;
    }

    int total_padding = 16 - pos;
    int j;
    for(j = 0; j < total_padding; j++){
        out[j] = '0';
    }

    for(i = 0; i < pos; i++){
        out[total_padding + i] = temp[pos - 1 - i];
    }
    out[16] = '\0';
}


//upside down binary tree - merkle tree
void hash_transaction(const Transaction *tx, char *out){
    char buf[512];
    int pos = 0;
    int i;

    for(i = 0; tx->sender[i] != '\0' && pos < 500; i++){
        buf[pos] = tx->sender[i];
        pos = pos + 1;
    }

    for(i = 0; tx->receiver[i] != '\0' && pos < 500; i++){
        buf[pos] = tx->receiver[i];
        pos = pos + 1;
    }

    long amount_cents = (long)(tx->amount * 100);
    char amount_str[32];
    int amt_pos = 0;

    if(amount_cents == 0){
        amount_str[0] = '0';
        amt_pos = 1;
    }
    else{
        long temp_amt = amount_cents;
        int neg = 0;
        if(temp_amt < 0){
            neg = 1;
            temp_amt = -temp_amt;
        }
        char amt_buf[32];
        int buf_pos = 0;
        while(temp_amt > 0){
            int digit = temp_amt % 10;
            amt_buf[buf_pos] = digit + '0';
            buf_pos = buf_pos + 1;
            temp_amt = temp_amt / 10;
        }
        if(neg){
            amount_str[0] = '-';
            amt_pos = 1;
        }
        int k;
        for(k = 0; k < buf_pos; k++){
            amount_str[amt_pos] = amt_buf[buf_pos - 1 - k];
            amt_pos = amt_pos + 1;
        }
    }
    amount_str[amt_pos] = '\0';

    for(i = 0; amount_str[i] != '\0' && pos < 500; i++){
        buf[pos] = amount_str[i];
        pos = pos + 1;
    }

    long ts = tx->timestamp;
    char ts_str[32];
    int ts_pos = 0;
    if(ts == 0){
        ts_str[0] = '0';
        ts_pos = 1;
    }
    else{
        long temp_ts = ts;
        char ts_buf[32];
        int tbuf_pos = 0;
        while(temp_ts > 0){
            int digit = temp_ts % 10;
            ts_buf[tbuf_pos] = digit + '0';
            tbuf_pos = tbuf_pos + 1;
            temp_ts = temp_ts / 10;
        }
        int k;
        for(k = 0; k < tbuf_pos; k++){
            ts_str[ts_pos] = ts_buf[tbuf_pos - 1 - k];
            ts_pos = ts_pos + 1;
        }
    }
    ts_str[ts_pos] = '\0';

    for(i = 0; ts_str[i] != '\0' && pos < 500; i++){
        buf[pos] = ts_str[i];
        pos = pos + 1;
    }

    buf[pos] = '\0';

    unsigned long h = djb2_hash(buf);
    hash_to_hex(h, out);
}

void compute_merkle_root(const Transaction txns[], int count, char *root_out){
    if(count == 0){
        hash_to_hex(djb2_hash("empty"), root_out);
        return;
    }

    char hashes[64][HashHexLen];
    int n = count;

    int i;
    for(i = 0; i < n; i++){
        hash_transaction(&txns[i], hashes[i]);
    }

    if(n % 2 != 0){
        strcpy(hashes[n], hashes[n - 1]);
        n = n + 1;
    }

    while(n > 1){
        int new_n = 0;
        i = 0;
        while(i < n){
            char combined[HashHexLen * 2];
            int cp = 0;
            int j;

            for(j = 0; j < 16; j++){
                combined[cp] = hashes[i][j];
                cp = cp + 1;
            }

            for(j = 0; j < 16; j++){
                combined[cp] = hashes[i + 1][j];
                cp = cp + 1;
            }
            combined[cp] = '\0';

            hash_to_hex(djb2_hash(combined), hashes[new_n]);
            new_n = new_n + 1;
            i = i + 2;
        }

        if(new_n > 1 && new_n % 2 != 0){
            strcpy(hashes[new_n], hashes[new_n - 1]);
            new_n = new_n + 1;
        }
        n = new_n;
    }

    strcpy(root_out, hashes[0]);
}


//block hash functions
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


//mining
void mine_block(Block *b){
    unsigned long target_starts_with = 0;
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


//blockchain functions
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


//wallet functions
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


//search functions
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


//main function
int main(void){
    Blockchain bc;
    blockchain_init(&bc);

    WalletList wallets;
    wallet_init(&wallets);
    wallet_update(&wallets, "Ritvik", 100.0);
    wallet_update(&wallets, "Kanha", 50.0);
    wallet_update(&wallets, "Faizan", 75.0);

    TransactionQueue pool;
    queue_init(&pool);

    int choice;
    do{
        printf("\n+==================================+\n");
        printf("|    BLOCKCHAIN SIMULATOR (C)      |\n");
        printf("+==================================+\n");
        printf("| 1. Add Transaction               |\n");
        printf("| 2. Mine Pending Transactions     |\n");
        printf("| 3. Display Blockchain Ledger     |\n");
        printf("| 4. Check Wallet Balances         |\n");
        printf("| 5. Verify Chain Integrity        |\n");
        printf("| 6. Tamper with a Block           |\n");
        printf("| 7. Search Transactions by User   |\n");
        printf("| 8. Find Block by Index           |\n");
        printf("| 9. Find Block by Hash Prefix     |\n");
        printf("| 0. Exit                          |\n");
        printf("+==================================+\n");
        printf("Choice: ");
        scanf("%d", &choice);

        switch(choice){

            case 1:{
                char s[MaxName], r[MaxName];
                double amt;
                printf("Sender  : "); scanf("%s", s);
                printf("Receiver: "); scanf("%s", r);
                printf("Amount  : "); scanf("%lf", &amt);

                WalletNode *we = wallet_find(&wallets, s);
                if(we == NULL || we->balance < amt){
                    printf("Insufficient balance!\n");
                    break;
                }

                Transaction tx;
                int i = 0;
                while(s[i] != '\0' && i < MaxName - 1){
                    tx.sender[i] = s[i];
                    i = i + 1;
                }
                tx.sender[i] = '\0';

                i = 0;
                while(r[i] != '\0' && i < MaxName - 1){
                    tx.receiver[i] = r[i];
                    i = i + 1;
                }
                tx.receiver[i] = '\0';

                tx.amount = amt;
                tx.timestamp = time(NULL);
                queue_enqueue(&pool, tx);
                printf("Transaction queued.\n");
                break;
            }

            case 2:{
                if(queue_is_empty(&pool)){
                    printf("No pending transactions.\n");
                    break;
                }

                Transaction batch[MaxTxPerBlock];
                int cnt = 0;
                while(queue_is_empty(&pool) == 0 && cnt < MaxTxPerBlock){
                    batch[cnt] = queue_dequeue(&pool);
                    cnt = cnt + 1;
                }

                blockchain_add_block(&bc, batch, cnt);

                int i;
                for(i = 0; i < cnt; i++){
                    wallet_update(&wallets, batch[i].sender, -batch[i].amount);
                    wallet_update(&wallets, batch[i].receiver, batch[i].amount);
                }
                break;
            }

            case 3:
                blockchain_print_ledger(&bc);
                break;

            case 4:
                wallet_print_all(&wallets);
                break;

            case 5:
                blockchain_verify(&bc);
                break;

            case 6:{
                int idx;
                char nr[MaxName];
                double na;
                printf("Block index to tamper: "); scanf("%d", &idx);
                printf("New receiver: "); scanf("%s", nr);
                printf("New amount  : "); scanf("%lf", &na);
                blockchain_tamper(&bc, idx, nr, na);
                printf("\nRunning verification...\n");
                blockchain_verify(&bc);
                break;
            }

            case 7:{
                char uname[MaxName];
                printf("Enter user name to search: ");
                scanf("%s", uname);
                search_transactions_by_user(&bc, uname);
                break;
            }

            case 8:{
                int idx;
                printf("Enter block index: ");
                scanf("%d", &idx);
                Block *found = find_block_by_index(&bc, idx);
                if(found != NULL){
                    printf("\n--- Block #%d Details ---\n", found->index);
                    printf("  Timestamp   : %ld\n", found->timestamp);
                    printf("  Prev Hash   : %s\n", found->Previous_hash);
                    printf("  Current Hash: %s\n", found->Current_hash);
                    printf("  Merkle Root : %s\n", found->merkle_root);
                    printf("  Nonce       : %lu\n", found->nonce);
                    printf("  Transactions:\n");
                    int i;
                    for(i = 0; i < found->tx_count; i++){
                        printf("    %s -> %s : %.2f coins\n",
                               found->transactions[i].sender,
                               found->transactions[i].receiver,
                               found->transactions[i].amount);
                    }
                }
                else{
                    printf("Block #%d not found.\n", idx);
                }
                break;
            }

            case 9:{
                char hpref[32];
                printf("Enter hash prefix to search: ");
                scanf("%s", hpref);
                Block *found = find_block_by_hash(&bc, hpref);
                if(found != NULL){
                    printf("\n--- Block Found #%d ---\n", found->index);
                    printf("  Hash: %s\n", found->Current_hash);
                    printf("  Timestamp: %ld\n", found->timestamp);
                    printf("  Nonce: %lu\n", found->nonce);
                }
                else{
                    printf("No block found with hash prefix '%s'.\n", hpref);
                }
                break;
            }

            case 0:
                printf("Goodbye!\n");
                break;

            default:
                printf("Invalid choice. Try again.\n");
        }
    }while(choice != 0);

    blockchain_free(&bc);
    wallet_free(&wallets);
    return 0;
}
