#ifndef BUFFER_H
#define BUFFER_H
#include <stdio.h>

typedef struct {
    unsigned char* data;
    size_t pos;
    size_t size;
    size_t max_size;
} Buffer;

int init_buffer_from_file(Buffer* buffer, FILE* file, size_t read_size);
void free_buffer(Buffer* buffer);
size_t read_chunk(Buffer* buffer, FILE* file);
ssize_t end_of_buffer(Buffer* buffer);
void print_buffer(const unsigned char* buffer, size_t size, int cols);
#endif
