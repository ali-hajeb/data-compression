#ifndef HUFFMAN_H
#define HUFFMAN_H
#include "bitio.h"
#include "minheap.h"

#include <stdint.h>
#include <stdio.h>
#include <corecrt.h>

typedef struct {
    uint32_t code;
    uint8_t length;
} Code;

size_t* count_run(FILE* file);
void generate_huffman_code(Code* code_table, uint32_t code, uint8_t depth, Node* node);
void encode(FILE* input_file, BitWriter* bit_writer, Code* code_table);
void decode(FILE* input_file, BitReader* bit_reader, unsigned char* frequency_table);
#endif
