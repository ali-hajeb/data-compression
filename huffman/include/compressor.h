#ifndef COMPRESSOR_H
#define COMPRESSOR_H
#include "minheap.h"

#include <stdio.h>


/*
* Function: fill_minheap
* ----------------------
* Creates a new node for every non-zero value in frequency table
* and inserts it to the min-heap.
*
* frequency_table: Pointer to the frequency table.
* priority_queue: Pointer to the min-heap object.
* max_count: If > 0, scales down the frequency
*
* returns: Count of inserted nodes.
*/
ssize_t fill_minheap(size_t* frequency_table, Heap* priority_queue, size_t max_count);

/*
* Function: compress
* ------------------
* Compresses the input file using huffman coding
*
* input_file: Pointer to the input_file
* output_file: Pointer to the output_file
*
* returns: If failed (0), On success (1)
*/
int compress(FILE* input_file, FILE* output_file);

/*
* Function: decompress
* ------------------
* Decompresses the input file using huffman coding
*
* input_file: Pointer to the input_file
* output_file: Pointer to the output_file
*
* returns: If failed (0), On success (1)
*/
int decompress(FILE* input_file, FILE* output_file);
void print_heap(Heap* heap, const char* title);
#endif
