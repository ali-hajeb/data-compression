#include "../include/constants.h"
#include "../include/bitio.h"
#include "../include/huffman.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
* Function: count_run
* -------------------
*  Calculates the occurance of every character
*
*  file: Pointer to the input file
*
*  returns: Array of frequencies
*/
size_t* count_run(FILE* file) {
    size_t* frequency_table = malloc(FREQUENCY_TABLE_SIZE * sizeof(size_t));
    if (frequency_table == NULL) {
        fprintf(stderr, "[ERROR]: count_run() {} -> Unable to allocate memory for frequency table!\n");
        return NULL;
    }
    // set every value to zero, in order to start counting occurance
    memset(frequency_table, 0, FREQUENCY_TABLE_SIZE * sizeof(size_t)); 

    char* read_buffer = malloc(READ_BUFFER_SIZE * sizeof(unsigned char));
    if (read_buffer == NULL) {
        fprintf(stderr, "[ERROR]: count_run() {} -> Unable to allocate memory for read buffer!\n");
        free(frequency_table);
        return NULL;
    }

    size_t read_bytes = 0;
    while ( (read_bytes = fread(read_buffer, sizeof(unsigned char), READ_BUFFER_SIZE, file)) != 0) {
        for (size_t i = 0; i < read_bytes; i++) {
            frequency_table[(unsigned char) read_buffer[i]]++;
        }
    }

    free(read_buffer);
    return frequency_table;
}

/*
* Function: generate_huffman_code
* -------------------------------
*  Generates binary codes based on the huffman tree for every leaf node
*
*  code_tabel: Array of codes to store the data
*  code: huffman binary code
*  depth: (default 0)
*  node: root node of the tree
*/
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

/*
* Function: encode
* ----------------
*  Encodes the input file using huffman encoding and saves the output to a file using BitWriter
*
*  input_file: Pointer to the file to be encoded
*  bit_writer: Pointer to the BitWriter object
*  code_table: Pointer to the code table
*/
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

/*
* Function: decode
* ----------------
*  Decodes a huffman file and save it to the output file.
*
*  output_file: Pointer to the output file.
*  bit_reader: Pointer to a BitReader object.
*  root: Pointer to the root node of the huffman tree.
*  total_bits: Number of encoded bits.
*/
void decode(FILE *output_file, BitReader *bit_reader, Node* root, size_t total_bits) {
    unsigned char out_buffer[OUTPUT_BUFFER_SIZE];
    size_t out_pos = 0;
    Node* current = root;

    while (bit_reader->bits_read < total_bits) {
        int bit = read_bits(bit_reader);
        if (bit == -1) break;
        current = bit ? current->r_node : current->l_node;
        // Leaf Node:
        if (current->l_node == NULL && current->r_node == NULL) { 
            out_buffer[out_pos++] = current->symbol;
            // Flush output_buffer
            if (out_pos == OUTPUT_BUFFER_SIZE) {
                fwrite(out_buffer, 1, OUTPUT_BUFFER_SIZE, output_file);
                out_pos = 0;
            }
            current = root;
        }
    }
    if (out_pos > 0) fwrite(out_buffer, 1, out_pos, output_file);
}
