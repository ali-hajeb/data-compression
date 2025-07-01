#ifndef RLE_H
#define RLE_H
#include "constants.h"

#include <stdio.h>

#define BASIC_COMPRESSION_LIMIT 255
#define ADVANCE_COMPRESSION_LIMIT 128

typedef enum {
    basic,
    advance
} CompressionMode;

typedef struct {
    unsigned char flag_byte;
    unsigned char buffer[OUTPUT_BUFFER_SIZE];
    unsigned short count_limit;
    FILE* file;
    CompressionMode compression_mode;
    size_t buffer_pos;
    size_t counter_pos;
    size_t flag_byte_count;
} RLEWriter;

typedef struct {
    unsigned char buffer[OUTPUT_BUFFER_SIZE];
    FILE* file;
    CompressionMode compression_mode;
    size_t buffer_pos;
} RLEReader;

int init_writer(RLEWriter* rle_writer, FILE* file, CompressionMode compression_mode);
int init_reader(RLEReader* rle_reader, FILE* file);
int write_rle(RLEWriter* rle_writer, unsigned char* chr);
size_t read_rle(RLEReader* rle_reader, unsigned char* counter_byte);
int flush_writer(RLEWriter* rle_writer);
int flush_reader(RLEReader* rle_reader);
ssize_t encode(FILE* input_file, RLEWriter* rle_writer);
ssize_t decode(FILE* input_file, RLEReader* rle_reader);

int flush_reader(RLEReader* rle_reader);
#endif
