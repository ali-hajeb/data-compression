#include "../include/compressor.h"
#include "../include/lz77.h"
#include "../include/utils.h"

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
               size_t compressor_buffer_size, size_t window_size) {
    if (input_file == NULL || output_file == NULL) {
        err("compress", "Input/output file is NULL!");
    }

    LZWriter lz_writer;
    int result = 99;
    result = init_writer(&lz_writer, output_file, writer_buffer_size, window_size);
    if (result < 1) {
        err("compress", "Failed to initiate writer!");
        return 0;
    }

    result = encode(&lz_writer, input_file, compressor_buffer_size);
    return result;
}

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
               size_t decompressor_buffer_size, size_t window_size) {
    if (input_file == NULL || output_file == NULL) {
        err("decompress", "Input/output file is NULL!");
    }

    LZReader lz_reader;
    int result = 99;
    result = init_reader(&lz_reader, output_file, reader_buffer_size, window_size);
    if (result < 1) {
        err("decompress", "Failed to initiate reader!");
        return 0;
    }

    result = decode(&lz_reader, input_file, decompressor_buffer_size);
    return result;
}
