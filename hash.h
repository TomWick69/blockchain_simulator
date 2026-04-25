#ifndef HASH_H
#define HASH_H

unsigned long djb2_hash(const char *str);
void hash_to_hex(unsigned long h, char *out);

#endif
