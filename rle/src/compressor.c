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
* compression_mode: "basic" or "advance" algorithm
*
* returns: If failed (0), On success (1)
*/
int compress(FILE* input_file, FILE* output_file, CompressionMode compression_mode) {    
    if (input_file == NULL || output_file == NULL) {
        err("compress", "Input/output file is NULL!");
    }

    RLEWriter rle_writer;
    init_writer(&rle_writer, output_file, compression_mode);

    int result = encode(input_file, &rle_writer);
    return result;
}

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
int decompress(FILE* input_file, FILE* output_file) {    
    if (input_file == NULL || output_file == NULL) {
        err("decompress", "Input/output file is NULL!");
    }

    CompressionMode compression_mode;
    int read_result = fread(&compression_mode, sizeof(CompressionMode), 1, input_file);
    if (read_result < 1 || (compression_mode != basic && compression_mode != advance)) {
        fprintf(stderr, "\n[ERROR]: decompress() {} -> File is corrupted!\n");
        return 0;
    }
    
    RLEReader rle_reader;
    init_reader(&rle_reader, output_file, compression_mode);

    int result = decode(input_file, &rle_reader);
    return result;
}
