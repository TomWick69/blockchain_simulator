#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"

void search_transactions_by_user(const Blockchain *bc, const char *name);
Block *find_block_by_index(const Blockchain *bc, int index);
Block *find_block_by_hash(const Blockchain *bc, const char *hash_prefix);

#endif
