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

/*
* Function: count_run
* -------------------
*  Calculates the occurance of every character
*
*  file: Pointer to the input file
*
*  returns: Array of frequencies
*/
size_t* count_run(FILE* file);

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
void generate_huffman_code(Code* code_table, uint32_t code, uint8_t depth, Node* node);

/*
* Function: encode
* ----------------
*  Encodes the input file using huffman encoding and saves the output to a file using BitWriter
*
*  input_file: Pointer to the file to be encoded
*  bit_writer: Pointer to the BitWriter object
*  code_table: Pointer to the code table
*/
void encode(FILE* input_file, BitWriter* bit_writer, Code* code_table);

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
void decode(FILE *output_file, BitReader *bit_reader, Node* root, size_t total_bits);
#endif
