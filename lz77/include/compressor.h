#ifndef COMPRESSOR_H
#define COMPRESSOR_H
#include <stdio.h>


/*
* Function: compress
* ------------------
* Compresses the input file using lz77 coding
*
* input_file: Pointer to the input_file
* output_file: Pointer to the output_file
* writer_buffer_size: Buffer size for writer (output buffer)
* compressor_buffer_size: Buffer size for chunck reader (input buffer)
* window_size: Sliding window size (dictionary size)
*
* returns: If failed (0), On success (1)
*/
int compress(FILE* input_file, FILE* output_file, size_t writer_buffer_size, 
               size_t compressor_buffer_size, size_t window_size);

/*
* Function: decompress
* ------------------
* Decompresses the input file using lz77 coding
*
* input_file: Pointer to the input_file
* output_file: Pointer to the outpug_file
* reader_buffer_size: Buffer size for reader (output buffer)
* decompressor_buffer_size: Buffer size for chunck reader (input buffer)
* window_size: Sliding window size (dictionary size)
*
* returns: If failed (0), On success (1)
*/
int decompress(FILE* input_file, FILE* output_file, size_t reader_buffer_size, 
               size_t decompressor_buffer_size, size_t window_size);
#endif

