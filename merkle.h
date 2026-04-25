#ifndef MERKLE_H
#define MERKLE_H

#include "types.h"

void hash_transaction(const Transaction *tx, char *out);
void compute_merkle_root(const Transaction txns[], int count, char *root_out);

#endif
