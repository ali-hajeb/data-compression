#ifndef HASH_H
#define HASH_H
#include "constants.h"
#include "buffer.h"

#include <stdint.h>
#include <stdio.h>

#define MAX_TABLE_SIZE (1UL << 16)
#define MAX_CHAIN_SIZE 64

typedef struct {
    int count;
    size_t positions[MAX_CHAIN_SIZE];
} HashItem;

typedef struct {
    HashItem* items;
} HashTable;

int init_hash_table(HashTable* hash_table);
unsigned int hash(const unsigned char* data, int length);
void update_hash_table(HashTable* hash_table, Buffer* buffer, size_t window_size);
size_t find_best_match(HashTable* hash_table, Buffer* buffer, size_t window_size, size_t* best_match_length);

#endif
