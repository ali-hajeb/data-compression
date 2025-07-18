#include "../include/compressor.h"
#include "../include/rle.h"
#include "../include/utils.h"

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
             CompressionMode compression_mode) {    
    if (input_file == NULL || output_file == NULL) {
        err("compress", "Input/output file is NULL!");
        return 0;
    }

    RLEWriter rle_writer;
    int error = init_writer(&rle_writer, output_file, writer_buffer_size, compression_mode);
    if (error == 0) {
        err("compress", "Unable to initiate RLEWriter");
        return 0;
    }

    int result = encode(input_file, &rle_writer, compressor_buffer_size);
    return result;
}

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
int decompress(FILE* input_file, FILE* output_file, size_t reader_buffer_size, size_t decompressor_buffer_size) {    
    if (input_file == NULL || output_file == NULL) {
        err("decompress", "Input/output file is NULL!");
        return 0;
    }

    CompressionMode compression_mode;
    int read_result = fread(&compression_mode, sizeof(unsigned char), 1, input_file);
    if (read_result < 1 || (compression_mode != basic && compression_mode != advance)) {
        fprintf(stderr, "\n[ERROR]: decompress() {} -> File is corrupted!\n");
        return 0;
    }
    
    RLEReader rle_reader;
    int error = init_reader(&rle_reader, output_file, reader_buffer_size, compression_mode);
    if (error == 0) {
        err("decompress", "Unable to initiate RLEReader");
        return 0;
    }

    int result = decode(input_file, &rle_reader, decompressor_buffer_size);
    return result;
}
