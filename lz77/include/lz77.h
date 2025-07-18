#ifndef LZ77_H
#define LZ77_H
#include "constants.h"

#include <stdio.h>


typedef struct {
    unsigned char* buffer;
    FILE* file;
    size_t buffer_pos;
    size_t buffer_size;
    size_t window_size;
} LZWriter;

typedef struct {
    unsigned char* buffer;
    unsigned char* dictionary;
    FILE* file;
    size_t buffer_pos;
    size_t buffer_size;
    size_t dict_pos;
    size_t dict_size;
} LZReader;

int init_writer(LZWriter* lz_writer, FILE* file, size_t buffer_size, size_t window_size);
int init_reader(LZReader* lz_reader, FILE* file, size_t buffer_size, size_t window_size);
ssize_t encode(LZWriter* lz_writer, FILE* input_file, size_t read_chunk_size);
ssize_t decode(LZReader* lz_reader, FILE* input_file, size_t read_chunk_size);
ssize_t flush_writer(LZWriter* lz_writer);
ssize_t flush_reader(LZReader* lz_reader);
size_t dictionary_push(LZReader* lz_reader, unsigned char* value);
#endif
