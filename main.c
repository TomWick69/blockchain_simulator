#include <stdio.h>
#include <string.h>
#include <time.h>
#include "types.h"
#include "queue.h"
#include "blockchain.h"
#include "wallet.h"
#include "search.h"

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
                printf("Exiting...\n");
                break;

            default:
                printf("Invalid choice. Try again.\n");
        }
    }while(choice != 0);

    blockchain_free(&bc);
    wallet_free(&wallets);
    return 0;
}
