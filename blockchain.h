#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include "types.h"

void compute_block_hash(const Block *b, char *out);
void mine_block(Block *b);
void blockchain_init(Blockchain *bc);
void blockchain_add_block(Blockchain *bc, Transaction txns[], int tx_count);
int blockchain_verify(const Blockchain *bc);
void blockchain_tamper(Blockchain *bc, int block_idx, const char *new_receiver, double new_amount);
void blockchain_print_ledger(const Blockchain *bc);
void blockchain_free(Blockchain *bc);

#endif
