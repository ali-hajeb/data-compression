#include "../include/buffer.h"

#include <stdio.h>
#include <stdlib.h>

int init_buffer_from_file(Buffer* buffer, FILE* file, size_t max_size) {
    if (buffer == NULL || file == NULL || max_size == 0) {
        fprintf(stderr, "\n[ERROR]: init_buffer_from_file() {} -> Required parameters are NULL!\n");
        return 0;
    }

    buffer->data = malloc(sizeof(unsigned char) * max_size);
    if (buffer->data == NULL) {
        fprintf(stderr, "\n[ERROR]: init_buffer_from_file() {} -> Unable to allocate memory for buffer!\n");
        return 0;
    }
    size_t read_bytes = fread(buffer->data, sizeof(unsigned char), max_size, file);
    buffer->size = read_bytes;
    buffer->max_size = max_size;
    buffer->pos = 0;
    return 1;
}

size_t read_chunk(Buffer* buffer, FILE* file) {
    size_t read_bytes = fread(buffer->data, sizeof(unsigned char), buffer->max_size, file);
    buffer->size = read_bytes;
    buffer->pos = 0;
    return read_bytes;
}

ssize_t end_of_buffer(Buffer* buffer) {
    return buffer->size - buffer->pos;
}

void free_buffer(Buffer* buffer) {
    if (buffer->data != NULL) {
        free(buffer->data);
    }
}

void print_buffer(const unsigned char* buffer, size_t size, int cols) {
    for (size_t i = 0; i < (size / cols + 1); i++) {
        for (int j = 0; j < cols && ((i * cols) + j) < size; j++) {
            printf("%02X  ", buffer[(i * cols) + j]);
        }
        printf("\n");
    }
}
