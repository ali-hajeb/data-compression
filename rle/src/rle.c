#include "../include/rle.h"
#include "../include/utils.h"

#include <stdio.h>
#include <stdlib.h>

int init_writer(RLEWriter* rle_writer, FILE* file, CompressionMode compression_mode) {
    if (file == NULL || rle_writer == NULL) {
        fprintf(stderr, "[ERROR]: init_writer() {} -> Required parameters are NULL!\n");
        return 0;
    }

    rle_writer->file = file;
    rle_writer->compression_mode = compression_mode;
    rle_writer->count_limit = compression_mode == basic ? BASIC_COMPRESSION_LIMIT : ADVANCE_COMPRESSION_LIMIT;
    rle_writer->counter_pos = -1;
    rle_writer->buffer_pos = 0;
    rle_writer->flag_byte = 0;
    rle_writer->flag_byte_count = 0;
    return 1;
}

int init_reader(RLEReader* rle_reader, FILE* file) {
    if (file == NULL || rle_reader == NULL) {
        fprintf(stderr, "[ERROR]: init_reader() {} -> Required parameters are NULL!\n");
        return 0;
    }

    rle_reader->file = file;
    int result = fread(&rle_reader->compression_mode, sizeof(CompressionMode), 1, file);
    if (result < 1 || (rle_reader->compression_mode != basic && rle_reader->compression_mode != advance)) {
        fprintf(stderr, "\n[ERROR]: init_reader() {} -> File is corrupted!\n");
        return 0;
    }
    rle_reader->buffer_pos = 0;
    return 1;
}

int write_rle(RLEWriter* rle_writer, unsigned char* chr) {
    if (rle_writer == NULL || chr == NULL) {
        fprintf(stderr, "\n[ERROR]: write_rle() {} -> Required parameters are NULL!\n");
        return 0;
    }

    size_t counter_padding = rle_writer->compression_mode ? 126 : 0;

    if (rle_writer->flag_byte_count == 0) {
        rle_writer->flag_byte = *chr;
    }

    if (rle_writer->flag_byte == *chr && rle_writer->flag_byte_count < rle_writer->count_limit) {
        rle_writer->flag_byte_count++;
    } else {
        if (rle_writer->flag_byte_count > 1 || rle_writer->compression_mode == basic) {
            rle_writer->buffer[rle_writer->buffer_pos++] = rle_writer->flag_byte_count + counter_padding;
            rle_writer->buffer[rle_writer->buffer_pos++] = rle_writer->flag_byte;
        } else {
            if (rle_writer->counter_pos > -1) {
                // Increase the counter for uncompressed sequence
                rle_writer->buffer[rle_writer->counter_pos]++;
                // Reset counter position for uncompressed sequence, if the counter is about to pass the limit
                if (rle_writer->buffer[rle_writer->counter_pos] + 1 >= rle_writer->count_limit) {
                    rle_writer->counter_pos = -1;
                }
                rle_writer->buffer[rle_writer->buffer_pos++] = rle_writer->flag_byte;
            } else {
                rle_writer->counter_pos = rle_writer->buffer_pos;
                rle_writer->buffer[rle_writer->buffer_pos++] = 1;
                rle_writer->buffer[rle_writer->buffer_pos++] = rle_writer->flag_byte;
            }
        }

        if (rle_writer->buffer_pos >= OUTPUT_BUFFER_SIZE) {
            int result = fwrite(rle_writer->buffer, sizeof(unsigned char), rle_writer->buffer_pos, rle_writer->file);
            if (result < rle_writer->buffer_pos) {
                fprintf(stderr, "\n[ERROR]: write_rle() {} -> Unable to flush the buffer!\n");
                return 0;
            }
            rle_writer->buffer_pos = 0;
            rle_writer->counter_pos = -1;
        }
        rle_writer->flag_byte = *chr;
        rle_writer->flag_byte_count = 1;
    }
    return 1;
}

size_t read_rle(RLEReader* rle_reader, unsigned char* counter_byte) {
    size_t count = *counter_byte;
    if (rle_reader->compression_mode == advance && *counter_byte >= ADVANCE_COMPRESSION_LIMIT) {
        count -= 126;
    }
    if (count <= 0) {
        fprintf(stderr, "\n[ERROR]: read_rle() {} -> Invalid value (count = %zu)\n", count);
        return 0;
    }

    if (rle_reader->buffer_pos + count >= OUTPUT_BUFFER_SIZE) {
        flush_reader(rle_reader);
    }

    size_t read_bytes = 0;
    
    if (rle_reader->compression_mode == basic) {
        for (int i = 0; i < count; i++) {
            rle_reader->buffer[rle_reader->buffer_pos++] = *(counter_byte + 1);
        }
        read_bytes++;
    } else {
        if (*counter_byte >= ADVANCE_COMPRESSION_LIMIT) {
            for (int i = 0; i < count; i++) {
                rle_reader->buffer[rle_reader->buffer_pos++] = *(counter_byte + 1);
            }
            read_bytes++;
        } else {
            for (int i = 0; i < count; i++) {
                rle_reader->buffer[rle_reader->buffer_pos++] = *(counter_byte + i);
                read_bytes++;
            }
        }

    }
    return read_bytes;
}

int flush_writer(RLEWriter* rle_writer) {
    if (rle_writer == NULL) {
        fprintf(stderr, "\n[ERROR]: flush_writer() {} -> RLEWriter is NULL!\n");
        return -1;
    }

    size_t processed = 0;

    if (rle_writer->flag_byte_count > 0) {
        processed = rle_writer->flag_byte_count;
        // Use a non-equal char in write_rle, so it ends the counter for the flag byte
        unsigned char _chr = rle_writer->flag_byte + 1;
        write_rle(rle_writer, &_chr);
    }
    if (rle_writer->buffer_pos >= OUTPUT_BUFFER_SIZE) {
        int result = fwrite(rle_writer->buffer, sizeof(unsigned char), rle_writer->buffer_pos, rle_writer->file);
        if (result < rle_writer->buffer_pos) {
            fprintf(stderr, "\n[ERROR]: flush_writer() {} -> Unable to flush the buffer!\n");
            return -1;
        }
        rle_writer->buffer_pos = 0;
        rle_writer->counter_pos = -1;
    }
    return processed;
}

int flush_reader(RLEReader* rle_reader) {
    if (rle_reader == NULL) {
        fprintf(stderr, "\n[ERROR]: flush_reader() {} -> RLEReader is NULL!\n");
        return -1;
    }

    size_t flushed_bytes = 0;
    
    if (rle_reader->buffer_pos > 0) {
        int result = fwrite(rle_reader->buffer, sizeof(unsigned char), rle_reader->buffer_pos, rle_reader->file);
        if (result < rle_reader->buffer_pos) {
            fprintf(stderr, "\n[ERROR]: flush_reader() {} -> Unable to flush the buffer!\n");
            return 0;
        }
        flushed_bytes = rle_reader->buffer_pos;
        rle_reader->buffer_pos = 0;
    }
    return flushed_bytes;
}

ssize_t encode(FILE* input_file, RLEWriter* rle_writer) {
    if (input_file == NULL) {
        fprintf(stderr, "[ERROR]: encode() {} -> File pointer is NULL!\n");
        return -1;
    }

    unsigned char* read_buffer = malloc(READ_BUFFER_SIZE * sizeof(unsigned char));
    if (read_buffer == NULL) {
        fprintf(stderr, "\n[ERROR]: encode() {} -> Unable to allocate memory for buffer!\n");
        return -1;
    }

    size_t read_bytes = 0;
    size_t file_size = get_file_size(input_file);
    size_t processed = 0;
    fseek(input_file, 0, SEEK_SET);

    if (fwrite(&rle_writer->compression_mode, sizeof(CompressionMode), 1, rle_writer->file) < 1) {
        fprintf(stderr, "\n[ERROR]: encode() {} -> Unable to write the compression mode to the file!\n");
        free(read_buffer);
        return -1;
    }

    while ((read_bytes = fread(read_buffer, sizeof(unsigned char), READ_BUFFER_SIZE, input_file)) != 0) {
        for (size_t i = 0; i < read_bytes; i++) {
            int result = write_rle(rle_writer, &read_buffer[i]);
            if (result == 0) {
                free(read_buffer);
                return -1;
            }
        }
        processed += read_bytes;
        if (processed % (100 * KB) == 0) {
            printf("\rProcessing: %zu/%zu bytes...", processed, file_size);
        }
    }

    int result = flush_writer(rle_writer);
    if (result < 0) {
        free(read_buffer);
        return -1;
    }

    long compressed_file_size = ftell(rle_writer->file);
    double compression_rate = (double) (file_size - compressed_file_size) / file_size * 100;
    printf("\rProcessing: %zu/%zu bytes. -> %ld bytes (%.2f%s)\n", processed, file_size, 
           compressed_file_size, compression_rate, "%");

    free(read_buffer);
    return processed;
}

ssize_t decode(FILE* input_file, RLEReader* rle_reader) {
    if (input_file == NULL) {
        fprintf(stderr, "[ERROR]: decode() {} -> File pointer is NULL!\n");
        return -1;
    }

    unsigned char* read_buffer = malloc(READ_BUFFER_SIZE * sizeof(unsigned char));
    if (read_buffer == NULL) {
        fprintf(stderr, "\n[ERROR]: decode() {} -> Unable to allocate memory for buffer!\n");
        return -1;
    }

    size_t read_bytes = 0;
    size_t file_size = get_file_size(input_file);
    size_t processed = 0;
    // Skip the first byte (compression mode byte)
    fseek(input_file, 1, SEEK_SET);

    while ((read_bytes = fread(read_buffer, sizeof(unsigned char), READ_BUFFER_SIZE, input_file)) != 0) {
        for (size_t i = 0; i < read_bytes; i++) {    
            size_t processed_bytes = read_rle(rle_reader, &read_buffer[i]);
            i += processed_bytes;
        }
        processed += read_bytes;
        if (processed % (100 * KB) == 0) {
            printf("\rProcessing: %zu/%zu bytes...", processed, file_size);
        }
    }

    int result = flush_reader(rle_reader);
    if (result < 0) {
        free(read_buffer);
        return -1;
    }

    long compressed_file_size = ftell(rle_reader->file);
    double compression_rate = (double) (file_size - compressed_file_size) / file_size * 100;
    printf("\rProcessing: %zu/%zu bytes. -> %ld bytes (%.2f%s)\n", processed, file_size, 
           compressed_file_size, compression_rate, "%");

    free(read_buffer);
    return processed;
}
