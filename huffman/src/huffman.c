#include "../include/constants.h"
#include "../include/bitio.h"
#include "../include/huffman.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t* count_run(FILE* file) {
    size_t* frequency_table = malloc(FREQUENCY_TABLE_SIZE * sizeof(size_t));
    if (frequency_table == NULL) {
        fprintf(stderr, "[ERROR]: count_run() {} -> Unable to allocate memory for frequency table!\n");
        return NULL;
    }
    memset(frequency_table, 0, FREQUENCY_TABLE_SIZE * sizeof(size_t)); // set every value to zero, in order to start counting occurance

    char* read_buffer = malloc(READ_BUFFER_SIZE * sizeof(unsigned char));
    if (read_buffer == NULL) {
        fprintf(stderr, "[ERROR]: count_run() {} -> Unable to allocate memory for read buffer!\n");
        return NULL;
    }

    size_t read_bytes;
    while ( (read_bytes = fread(read_buffer, sizeof(unsigned char), READ_BUFFER_SIZE, file)) != 0) {
        for (size_t i = 0; i < read_bytes; i++) {
            frequency_table[(unsigned char) read_buffer[i]]++;
        }
    }

    free(read_buffer);
    return frequency_table;
}

void generate_huffman_code(Code* code_table, uint32_t code, uint8_t depth, Node* node) {
    if (node == NULL) return;
    if (node->l_node == NULL && node->r_node == NULL) {
        code_table[node->symbol].code = code; 
        code_table[node->symbol].length = depth; 
        return;
    }

    if (node->r_node != NULL) {
        generate_huffman_code(code_table, (code << 1) | 1, depth + 1, node->r_node);
    }
    if (node->l_node != NULL) {
        generate_huffman_code(code_table, (code << 1), depth + 1, node->l_node);
    }
}

void encode(FILE* input_file, BitWriter* bit_writer, Code* code_table) {
    unsigned char read_buffer[READ_BUFFER_SIZE];
    size_t bytes_read = 0;
    fseek(input_file, 0, SEEK_SET);

    while((bytes_read = fread(read_buffer, 1, READ_BUFFER_SIZE, input_file)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            unsigned char symbol = read_buffer[i];
            if (code_table[symbol].length > 0) {
                write_bits(bit_writer, code_table[symbol].code, code_table[symbol].length);
            }
        }
    }

    flush_writer(bit_writer);
}
