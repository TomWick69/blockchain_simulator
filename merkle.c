#include <string.h>
#include "types.h"
#include "hash.h"
#include "merkle.h"

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
