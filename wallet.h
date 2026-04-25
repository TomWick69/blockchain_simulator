#ifndef WALLET_H
#define WALLET_H

#include "types.h"

void wallet_init(WalletList *wl);
WalletNode *wallet_find(WalletList *wl, const char *name);
void wallet_update(WalletList *wl, const char *name, double delta);
void wallet_print_all(const WalletList *wl);
void wallet_free(WalletList *wl);

#endif
