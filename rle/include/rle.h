#ifndef RLE_H
#define RLE_H
#include <stdio.h>

#define BASIC_COMPRESSION_LIMIT 255
#define ADVANCE_COMPRESSION_LIMIT 128

typedef enum {
    basic,
    advance
} CompressionMode;

typedef struct {
    unsigned char flag_byte;
    unsigned char* buffer;
    unsigned short count_limit;
    FILE* file;
    CompressionMode compression_mode;
    size_t buffer_pos;
    size_t buffer_size;
    ssize_t counter_pos;
    size_t flag_byte_count;
} RLEWriter;

typedef struct {
    unsigned char* buffer;
    FILE* file;
    CompressionMode compression_mode;
    size_t buffer_pos;
    size_t buffer_size;
} RLEReader;

/*
* Function: init_writer
* ---------------------
*  Initiates the RLEWriter object.
*
*  rle_writer: Pointer to the RLEWriter to initiate.
*  file: Pointer to the output file.
*  writer_buffer_size: RLEWriter buffer (output buffer) size 
*  compression_mode: Compression algorithm ('basic' or 'advance').
*
*  returns: If failed (0), on success (1)
*/
int init_writer(RLEWriter* rle_writer, FILE* file, size_t writer_buffer_size, CompressionMode compression_mode);

/*
* Function: init_reader
* ---------------------
*  Initiates the RLEReader object.
*
*  rle_reader: Pointer to the RLEReader to initiate.
*  file: Pointer to the output file.
*  reader_buffer_size: RLEReader buffer (output buffer) size 
*  compression_mode: Compression algorithm ('basic' or 'advance').
*
*  returns: If failed (0), on success (1)
*/
int init_reader(RLEReader* rle_reader, FILE* file, size_t reader_buffer_size, CompressionMode compression_mode);

/*
* Function: write_rle
* -------------------
*  Encodes and write RLE for the given character.
*
*  rle_writer: Pointer to the initiated RLEWriter.
*  chr: Pointer to the character from file.
*
*  returns: If failed (0), on success (1).
*/
int write_rle(RLEWriter* rle_writer, unsigned char* chr);

/*
* Function: read_rle
* -------------------
*  Decodes and write uncompressed data for the given compressed stream.
*
*  rle_reader: Pointer to the initiated RLEReader.
*  counter_byter: Pointer to the counter byte in compressed stream.
*
*  returns: Decoded bytes count.
*/
size_t read_rle(RLEReader* rle_reader, unsigned char* counter_byte);

/*
* Function: flush_writer
* ----------------------
*  Flushes the remaining data in buffer to the output file.
*
*  rle_writer: Pointer to the initiated RLEWriter
*
*  returns: Written bytes count.
*/
int flush_writer(RLEWriter* rle_writer);

/*
* Function: flush_reader
* ----------------------
*  Flushes the remaining data in buffer to the output file.
*
*  rle_readaer: Pointer to the initiated RLEReader
*
*  returns: Written bytes count.
*/
int flush_reader(RLEReader* rle_reader);

/*
* Function: encode
* ----------------
*  Encodes file using RLE technique.
*
*  input_file: Pointer to the input file.
*  rle_writer: Pointer to the initiated RLEWriter.
*  chunk_size: Input buffer size
*
*  returns: Encoded bytes count. If failed (-1).
*/
ssize_t encode(FILE* input_file, RLEWriter* rle_writer, size_t chunk_size);

/*
* Function: decode
* ----------------
*  Decodes file using RLE technique.
*
*  input_file: Pointer to the input file.
*  rle_reader: Pointer to the initiated RLEReader.
*  chunk_size: Input buffer size
*
*  returns: Decoded bytes count. If failed (-1).
*/
ssize_t decode(FILE* input_file, RLEReader* rle_reader, size_t chunk_size);

/*
* Function: print_buffer
* ----------------------
*  Prints HEX values of buffer content.
*
*  rle_writer: Pointer to the initiated RLEWriter.
*  cols: Number of Columns.
*/
void print_buffer(RLEWriter* rle_writer, int cols);
#endif
