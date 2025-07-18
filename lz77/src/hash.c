#include "../include/hash.h"
#include "../include/buffer.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int init_hash_table(HashTable* hash_table) {
    hash_table->items = malloc(MAX_TABLE_SIZE * sizeof(HashItem));
    if (hash_table->items == NULL) {
        fprintf(stderr, "\n[ERROR]: init_hash_table() {} -> Unable to allocate memory for the hash table!\n");
        return 0;
    }
    for (size_t i = 0; i < MAX_TABLE_SIZE; i++) {
        hash_table->items[i].count = 0;
    }
    return 1;
}

unsigned int hash(const unsigned char* data, int length) {
    // FNV-1a offset basis
    unsigned int hash_value = 2166136261u;
    for (int i = 0; i < length && i < 2; i++) {
        hash_value = (hash_value * 16777619u) ^ data[i];
    }
    return hash_value % MAX_TABLE_SIZE;
}

void update_hash_table(HashTable* hash_table, Buffer* buffer, size_t window_size) {
    size_t pos = buffer->pos;
    unsigned int hash_value = hash(buffer->data + pos, 2);
    HashItem* hash_item = &hash_table->items[hash_value];

    if (hash_item->count >= MAX_CHAIN_SIZE) {
        memmove(hash_item->positions, hash_item->positions + 1, (MAX_CHAIN_SIZE - 1) * sizeof(size_t));
        hash_item->count--;
    }

    hash_item->positions[hash_item->count++] = pos;

    while (hash_item->count > 0 && pos - hash_item->positions[0] > window_size) {
        memmove(hash_item->positions, hash_item->positions + 1, (hash_item->count - 1) * sizeof(size_t));
        hash_item->count--;
    }
}

size_t find_best_match(HashTable* hash_table, Buffer* buffer, size_t window_size, size_t* best_match_length) {
    *best_match_length = 0;
    size_t pos = buffer->pos;
    size_t data_size = buffer->size;
    size_t best_match_pos = pos;

    if (pos + 1 >= data_size) return best_match_pos;

    unsigned int hash_value = hash(buffer->data + pos, 2);
    HashItem* hash_item = &hash_table->items[hash_value];

    for (int i = 0; i < hash_item->count; i++) {
        size_t prev_pos = hash_item->positions[i];
        if (pos - prev_pos > window_size || pos <= prev_pos) {
            continue;
        }

        int match_length = 0;
        // printf("------|match at %zu\n", prev_pos);
        while (pos + match_length < data_size
            && prev_pos + match_length < pos
            && buffer->data[pos + match_length] == buffer->data[prev_pos + match_length]
            && match_length < 255) {
            // printf("[%zu]: %02X == %02X :[%zu]\n", pos + match_length, data[pos + match_length], data[prev_pos + match_length], prev_pos + match_length);
            match_length++;
        }

        if (match_length >= 2 && match_length > *best_match_length) {
            best_match_pos = pos - prev_pos;
            *best_match_length = match_length;
        }
    }
    return best_match_pos;
}
