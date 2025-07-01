#ifndef COMPRESSOR_H
#define COMPRESSOR_H
#include "constants.h"
#include "rle.h"

#include <stdio.h>


/*
* Function: compress
* ------------------
* Compresses the input file using huffman coding
*
* input_file: Pointer to the input_file
* output_file: Pointer to the output_file
* compression_mode: "basic" or "advance" algorithm
*
* returns: If failed (0), On success (1)
*/
int compress(FILE* input_file, FILE* output_file, CompressionMode compression_mode);

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
#endif
