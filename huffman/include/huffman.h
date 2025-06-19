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

typedef struct {
    unsigned char symbol;
    unsigned char frequency;
} HeaderFrequencyTable;

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
* Function: write_file_header
* ---------------------------
*  Writes header information to the output file
*
*  output_file: Pointer to the output file
*  frequency_table: Pointer to the frequency table
*
*  returns: If failed (0), on success (1)
*/
int write_file_header(FILE* output_file, size_t* frequency_table);

/*
* Function: read_file_header
* --------------------------
*  Reads the header information of compressed file.
*
*  input_file: Pointer to the compressed file
*  bit_count: Pointer to the variable storing total bit count
*
*  returns: Frequency table
*/
size_t* read_file_header(FILE* input_file, size_t* bit_count);

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
*
*  returns: If failed (0), On success (1)
*/
int encode(FILE* input_file, BitWriter* bit_writer, Code* code_table);

/*
* Function: decode
* ----------------
*  Decodes a huffman file and save it to the output file.
*
*  output_file: Pointer to the output file.
*  bit_reader: Pointer to a BitReader object.
*  root: Pointer to the root node of the huffman tree.
*  total_bits: Number of encoded bits.
*
*  returns: If failed (0), on success (1)
*/
int decode(FILE *output_file, BitReader *bit_reader, Node* root, size_t total_bits);
#endif
