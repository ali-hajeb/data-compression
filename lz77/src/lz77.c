#include "../include/lz77.h"
#include "../include/buffer.h"
#include "../include/hash.h"
#include "../include/utils.h"
#include "../include/constants.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


void compare_buffers(const unsigned char* buff1, size_t buff1_size,
                     const unsigned char* buff2, size_t buff2_size, int cols) {
    size_t min_size = buff1_size > buff2_size ? buff2_size : buff1_size;
    size_t max_size = buff1_size < buff2_size ? buff2_size : buff1_size;
    size_t row_limit = max_size / cols + 1;
    printf("\r\n");
    for (size_t i = 0; i < row_limit; i++) {
        size_t base_idx = i * cols;
        short k = 0;
        for (int j = 0; j < cols && base_idx + j < buff1_size; j++) {
            printf("%02X  ", buff1[base_idx + j]);
            k += 4;
        }
        printf("%*s", 40 - k, "||  ");
        for (int j = 0; j < cols && base_idx + j < buff2_size; j++) {
            printf("%02X  ", buff2[base_idx + j]);
        }
        printf("\r\n");
    }
}

int init_writer(LZWriter* lz_writer, FILE* file, size_t buffer_size, size_t window_size) {
    if (lz_writer == NULL || file == NULL || buffer_size == 0 || window_size == 0) {
        fprintf(stderr, "\n[ERROR]: init_writer() {} -> Required parameters are NULL!\n");
        return 0;
    }

    lz_writer->file = file;
    lz_writer->buffer = malloc(buffer_size * sizeof(unsigned char));
    if (lz_writer->buffer == NULL) {
        fprintf(stderr, "\n[ERROR]: init_writer() {} -> Unable to allocate memory for buffer!\n");
        return 0;
    }
    // memset(lz_writer->buffer, 0, lz_writer->buffer_size * sizeof(unsigned char));
    lz_writer->buffer_pos = 0;
    lz_writer->buffer_size = buffer_size;
    lz_writer->window_size = window_size;
    return 1;
}

int init_reader(LZReader* lz_reader, FILE* file, size_t buffer_size, size_t window_size) {
    if (lz_reader == NULL || file == NULL) {
        fprintf(stderr, "\n[ERROR]: init_reader() {} -> Required parameters are NULL!\n");
        return 0;
    }
    lz_reader->file = file;
    lz_reader->buffer = malloc(buffer_size * sizeof(unsigned char));
    lz_reader->dictionary = malloc(window_size * sizeof(unsigned char));
    if (lz_reader->buffer == NULL || lz_reader->dictionary == NULL) {
        fprintf(stderr, "\n[ERROR]: init_writer() {} -> Unable to allocate memory for buffer!\n");
        return 0;
    }
    // memset(lz_reader->buffer, 0, lz_reader->buffer_size * sizeof(unsigned char));
    // memset(lz_reader->dictionary, 0xFF, window_size * sizeof(unsigned char));
    lz_reader->buffer_pos = 0;
    lz_reader->buffer_size = buffer_size;
    lz_reader->dict_pos = 0;
    lz_reader->dict_size = window_size;
    return 1;
}

void free_writer(LZWriter* lz_writer) {
    if (lz_writer) {
        if (lz_writer->buffer) free(lz_writer->buffer);
        free(lz_writer);
    }
}

void free_reader(LZReader* lz_reader) {
    if (lz_reader) {
        if (lz_reader->buffer) free(lz_reader->buffer);
        if (lz_reader->dictionary) free(lz_reader->dictionary);
        free(lz_reader);
    }
}

size_t dictionary_push(LZReader* lz_reader, unsigned char* value) {
    size_t pos = lz_reader->dict_pos;
    lz_reader->dictionary[pos] = *value;
    lz_reader->dict_pos = (lz_reader->dict_pos + 1) % lz_reader->dict_size;
    return pos;
}

ssize_t write_lz(LZWriter* lz_writer, HashTable* hash_table, Buffer* buffer) {
    if (lz_writer == NULL || buffer == NULL || buffer->data == NULL) {
        fprintf(stderr, "\n[ERROR]: write_lz() {} -> Required parameters are NULL!\n");
        return -1;
    }

    if (lz_writer->buffer_pos + 3 >= lz_writer->buffer_size) {
        ssize_t result = flush_writer(lz_writer);
        if (result < 0) {
            return -1;
        }
    }

    update_hash_table(hash_table, buffer, lz_writer->window_size);

    size_t pos = buffer->pos;
    size_t best_match_length = 0;
    size_t best_match_pos = find_best_match(hash_table, buffer, lz_writer->window_size, &best_match_length);

    if (best_match_length >= 2) {
        lz_writer->buffer[lz_writer->buffer_pos++] = (best_match_pos) & 0xFF; 
        lz_writer->buffer[lz_writer->buffer_pos++] =  (best_match_pos >> 8) & 0xFF; 
        lz_writer->buffer[lz_writer->buffer_pos++] = (uint8_t) best_match_length; 
        return best_match_length;
    } else {
        lz_writer->buffer[lz_writer->buffer_pos++] = 0; 
        lz_writer->buffer[lz_writer->buffer_pos++] = 0; 
        lz_writer->buffer[lz_writer->buffer_pos++] = buffer->data[pos]; 
        return 1;
    }
}

ssize_t read_lz(Buffer* buffer, LZReader* lz_reader) {
    if (lz_reader == NULL || buffer == NULL || buffer->data == NULL) {
        fprintf(stderr, "\n[ERROR]: reader_lz() {} -> Required parameters are NULL!\n");
        return -1;
    }

    size_t pos = buffer->pos;
    uint8_t lsb = buffer->data[buffer->pos++];
    uint8_t msb = buffer->data[buffer->pos++];
    uint16_t offset = (msb << 8) | lsb;
    if (offset > 0) {
        // Match
        uint8_t length = buffer->data[buffer->pos++];
        if (lz_reader->buffer_pos + length >= lz_reader->buffer_size) {
            ssize_t result = flush_reader(lz_reader);
            if (result < lz_reader->buffer_pos) {
                return -1;
            }
        }
        for (uint8_t i = 0; i < length; i++) {
            size_t dict_idx = lz_reader->dict_pos - offset;
            lz_reader->buffer[lz_reader->buffer_pos++] = lz_reader->dictionary[dict_idx];
            dictionary_push(lz_reader, &lz_reader->dictionary[dict_idx]);
        }
    } else {
        // Literal
        if (lz_reader->buffer_pos + 1 >= lz_reader->dict_size) {
            ssize_t result = flush_reader(lz_reader);
            if (result < lz_reader->buffer_pos) {
                return -1;
            }
        }
        lz_reader->buffer[lz_reader->buffer_pos++] = buffer->data[buffer->pos];
        dictionary_push(lz_reader, &buffer->data[buffer->pos++]);
    }

    return buffer->pos - pos;
}

ssize_t flush_writer(LZWriter* lz_writer) {
    if (lz_writer == NULL) {
        fprintf(stderr, "\n[ERROR]: flush_writer() {} -> Required parameters are NULL!\n");
        return -1;
    }

    ssize_t result = fwrite(lz_writer->buffer, sizeof(unsigned char), lz_writer->buffer_pos, lz_writer->file);
    if (result < lz_writer->buffer_pos) {
        fprintf(stderr, "\n[ERROR]: flush_writer() {} -> Unable to flush the writer!\n");
        return -1;
    }
    // memset(lz_writer->buffer, 0, lz_writer->buffer_size * sizeof(unsigned char));
    lz_writer->buffer_pos = 0;
    return result;
}

ssize_t flush_reader(LZReader* lz_reader) {
    if (lz_reader == NULL) {
        fprintf(stderr, "\n[ERROR]: flush_reader() {} -> Required parameters are NULL!\n");
        return -1;
    }

    ssize_t result = fwrite(lz_reader->buffer, sizeof(unsigned char), lz_reader->buffer_pos, lz_reader->file);
    if (result < lz_reader->buffer_pos) {
        fprintf(stderr, "\n[ERROR]: flush_reader() {} -> Unable to flush the reader!\n");
        return -1;
    }
    // memset(lz_reader->buffer, 0, lz_reader->buffer_size * sizeof(unsigned char));
    lz_reader->buffer_pos = 0;
    return result;
}

ssize_t encode(LZWriter* lz_writer, FILE* input_file, size_t read_chunk_size) {
    if (lz_writer == NULL || input_file == NULL) {
        fprintf(stderr, "\n[ERROR]: encode() {} -> Required parameters are NULL!\n");
        return -1;
    }

    HashTable hash_table;
    init_hash_table(&hash_table);

    Buffer buffer;
    init_buffer_from_file(&buffer, input_file, read_chunk_size);

    size_t file_size = get_file_size(input_file);
    size_t processed = 0;
    clock_t start_time = clock();
    fseek(input_file, 0, SEEK_SET);

    while (read_chunk(&buffer, input_file) != 0) {
        while (end_of_buffer(&buffer) > 0) {
            size_t result = write_lz(lz_writer, &hash_table, &buffer);
            if (result < 1) {
                fprintf(stderr, "\n[ERROR]: encode() {} -> Unable to write the encoded data into the buffer!\n");
                free_buffer(&buffer);
                return -1;
            }
            buffer.pos += result;
        }

        processed += buffer.size;
        if (processed % (100 * KB) == 0) {
            printf("\rProcessing: %zu/%zu bytes...", processed, file_size);
        }
    }    

    if (lz_writer->buffer_pos > 0) {
        int result = flush_writer(lz_writer);
        if (result < 0) {
            free_buffer(&buffer);
            return -1;
        }
    }

    clock_t end_time = clock();
    long compressed_file_size = ftell(lz_writer->file);
    int size_diff = file_size - compressed_file_size;
    double compression_rate = (double) abs(size_diff) / file_size * 100;
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("\rFinished processing (%f s): %zu bytes -> %ld bytes (%s%.2f%%)\n", time_spent, file_size, 
           compressed_file_size, size_diff > 0 ? "-" : "+", compression_rate);
    
    free_buffer(&buffer);
    return processed;
}


ssize_t decode(LZReader* lz_reader, FILE* input_file, size_t read_chunk_size) {
    if (lz_reader == NULL || input_file == NULL) {
        fprintf(stderr, "\n[ERROR]: decode() {} -> Required parameters are NULL!\n");
        return -1;
    }

    Buffer buffer;
    init_buffer_from_file(&buffer, input_file, read_chunk_size);

    size_t file_size = get_file_size(input_file);
    size_t processed = 0;
    fseek(input_file, 0, SEEK_SET);
    clock_t start_time = clock();

    while (read_chunk(&buffer, input_file) != 0) {
        size_t bytes_left = 0;
        while ((bytes_left = end_of_buffer(&buffer)) > 0) {
            if (bytes_left < 3) {
                fseek(input_file, -bytes_left, SEEK_CUR);
                break;
            }

            ssize_t result = read_lz(&buffer, lz_reader);
            if (result < 1) {
                fprintf(stderr, "\n[ERROR]: decode() {} -> Unable to write the decoded data into the buffer!\n");
                free_buffer(&buffer);
                return -1;
            }
        }

        processed += buffer.size;
        if (processed % (100 * KB) == 0) {
            printf("\rProcessing: %zu/%zu bytes...", processed, file_size);
        }
    }

    if (lz_reader->buffer_pos > 0) {
        int result = flush_reader(lz_reader);
        if (result < 0) {
            free_buffer(&buffer);
            return -1;
        }
    }

    clock_t end_time = clock();
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    long compressed_file_size = ftell(lz_reader->file);
    printf("\rFinished Processing (%f s): %zu bytes -> %ld bytes.\n", time_spent, file_size, compressed_file_size);

    free_buffer(&buffer);
    return processed;
}
