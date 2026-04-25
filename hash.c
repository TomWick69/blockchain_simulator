#include <string.h>
#include "hash.h"

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
