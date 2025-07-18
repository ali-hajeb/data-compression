#ifndef HUFFMAN_H
#define HUFFMAN_H
#include "bitio.h"
#include "minheap.h"

#include <stdint.h>
#include <stdio.h>

typedef struct node {
    unsigned char symbol;
    struct node* r_node;
    struct node* l_node;
    size_t frequency;
} Node;

typedef struct {
    uint8_t length;
    uint32_t code;
} Code;

typedef struct {
    unsigned char symbol;
    unsigned char frequency;
} HeaderFrequencyTable;

/*
* Function: get_list_size
* -----------------------
*  Returns the number of non-zero values. Also can obtain the maximum value.
*
*  list: Pointer to the list array.
*  max_value: Pointer to the max_value variable. (Can be NULL).
*
*  returns: Count of non-zero values.
*/
size_t get_list_size(size_t* list, size_t* max_value);

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
* Function compare_nodes
* ----------------------
*  Compares the frequency of two nodes
*
*  a: Pointer to Node 1
*  b: Pointer to Node 2
*
*  returns: The difference of the nodes priority
*/
int compare_nodes(const void* a, const void* b);

/*
* Function combine_nodes
* ----------------------
*  Combines the nodes and returns a new node
*
*  n1: First node
*  n2: Second node
*
*  returns: New node with 2 childeren
*/
Node* combine_nodes(Node* n1, Node* n2);

/*
* Function build_tree
* -------------------
*  Builds the binary tree of the min-heap
*
*  heap: Pointer to the min-heap
*
*  returns: A pointer to the root of the tree
*/
Node* build_tree(Heap* heap);

/*
* Function: free_tree
* -------------------
*  Frees the allocated memory for the tree (Recursively).
*
*  root: Pointer to the root of the tree
*/
void free_tree(Node* root);

/*
* Function: print_tree
* --------------------
*  Prints the tree (Recursively) .
*
*  root: Pointer to the root of the tree
*  indent: Number of space indentation after each branch
*/

void print_tree(Node* root, int indent); 

/*
* Function: free_heap_nodes
* -------------------------
*  Iterates over the nodes in min-heap object
*  and deletes them from the memory.
*
*  heap: Pointer to the min-heap object.
*
*  returns: Number of freed nodes.
*/
size_t free_heap_nodes(Heap* heap);

/*
* Function: scale_down
* --------------------
*  Returns an scaled down value.
*
*  value: The value to be scaled down
*  scale: The base value
*
*  returns: Scaled down value.
*/
size_t scale_down(size_t value, size_t scale);

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
*  list_size: Pointer to a variable to store frequency table size
*  bit_padding: Pointer to the variable storing remaining bit count
*
*  returns: Frequency table
*/
size_t* read_file_header(FILE* input_file, size_t* list_size, int* bit_padding);
// size_t* read_file_header(FILE* input_file, size_t* list_size, size_t* bit_count);

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
*  bit_padding: Number of encoded bits in the last byte of the file
*
*  returns: If failed (0), on success (1)
*/
int decode(FILE *output_file, BitReader *bit_reader, Node* root, int bit_padding);

// int decode(FILE *output_file, BitReader *bit_reader, Node* root, size_t total_bits);
#endif
