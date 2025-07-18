#ifndef COMPRESSOR_H
#define COMPRESSOR_H
#include "rle.h"

#include <stdio.h>


/*
* Function: compress
* ------------------
* Compresses the input file using huffman coding
*
* input_file: Pointer to the input_file
* output_file: Pointer to the output_file
* writer_buffer_size: RLEWriter buffer (output buffer) size 
* compressor_buffer_size: Compressor input buffer size
* compression_mode: "basic" or "advance" algorithm
*
* returns: If failed (0), On success (1)
*/
int compress(FILE* input_file, FILE* output_file, size_t writer_buffer_size, size_t compressor_buffer_size,
             CompressionMode compression_mode);

/*
* Function: decompress
* ------------------
* Decompresses the input file using huffman coding
*
* input_file: Pointer to the input_file
* output_file: Pointer to the output_file
* reader_buffer_size: RLEReader buffer (output buffer) size 
* decompressor_buffer_size: Compressor input buffer size
*
* returns: If failed (0), On success (1)
*/
int decompress(FILE* input_file, FILE* output_file, size_t reader_buffer_size, size_t decompressor_buffer_size);    
#endif
