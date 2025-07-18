#include "../include/constants.h"
#include "../include/rle.h"
#include "../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

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
int init_writer(RLEWriter* rle_writer, FILE* file, size_t writer_buffer_size, CompressionMode compression_mode) {
    if (file == NULL || rle_writer == NULL || writer_buffer_size == 0) {
        fprintf(stderr, "[ERROR]: init_writer() {} -> Required parameters are NULL!\n");
        return 0;
    }

    rle_writer->file = file;
    rle_writer->compression_mode = compression_mode;
    rle_writer->count_limit = compression_mode == basic ? BASIC_COMPRESSION_LIMIT : ADVANCE_COMPRESSION_LIMIT;
    rle_writer->counter_pos = -1;
    rle_writer->buffer_size = writer_buffer_size * sizeof(unsigned char);
    rle_writer->buffer = malloc(rle_writer->buffer_size);
    if (rle_writer->buffer == NULL) {
        fprintf(stderr, "[ERROR]: init_writer() {} -> Unable to allocate memory for the buffer!\n");
        return 0;
    }
    rle_writer->buffer_pos = 0;
    rle_writer->flag_byte = 0;
    rle_writer->flag_byte_count = 0;
    return 1;
}

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
int init_reader(RLEReader* rle_reader, FILE* file, size_t reader_buffer_size, CompressionMode compression_mode) {
    if (file == NULL || rle_reader == NULL) {
        fprintf(stderr, "[ERROR]: init_reader() {} -> Required parameters are NULL!\n");
        return 0;
    }

    rle_reader->file = file;
    rle_reader->compression_mode = compression_mode;
    rle_reader->buffer_size = reader_buffer_size * sizeof(unsigned char);
    rle_reader->buffer = malloc(rle_reader->buffer_size);
    if (rle_reader->buffer == NULL) {
        fprintf(stderr, "[ERROR]: init_reader() {} -> Unable to allocate memory for the buffer!\n");
        return 0;
    }
    rle_reader->buffer_pos = 0;
    return 1;
}

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
        rle_writer->counter_pos = -1;
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

        if (rle_writer->buffer_pos >= rle_writer->buffer_size) {
            size_t result = fwrite(rle_writer->buffer, sizeof(unsigned char), rle_writer->buffer_pos, rle_writer->file);
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
size_t read_rle(RLEReader* rle_reader, unsigned char* counter_byte) {
    int count = *counter_byte;
    if (rle_reader->compression_mode == advance && *counter_byte >= ADVANCE_COMPRESSION_LIMIT) {
        count -= 126;
    }
    if (count <= 0) {
        fprintf(stderr, "\n[ERROR]: read_rle() {} -> Invalid value (count = %d)\n", count);
        return 0;
    }

    if (rle_reader->buffer_pos + count >= rle_reader->buffer_size) {
        flush_reader(rle_reader);
    }

    size_t read_bytes = 0;
    
    if (rle_reader->compression_mode == basic || *counter_byte >= ADVANCE_COMPRESSION_LIMIT) {
        for (int i = 0; i < count; i++) {
            rle_reader->buffer[rle_reader->buffer_pos++] = *(counter_byte + 1);
        }
        read_bytes++;
    } else {
        for (int i = 1; i <= count; i++) {
            rle_reader->buffer[rle_reader->buffer_pos++] = *(counter_byte + i);
            read_bytes++;
        }

    }
    return read_bytes;
}

/*
* Function: flush_writer
* ----------------------
*  Flushes the remaining data in buffer to the output file.
*
*  rle_writer: Pointer to the initiated RLEWriter
*
*  returns: Written bytes count.
*/
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
    if (rle_writer->buffer_pos > 0) {
        size_t result = fwrite(rle_writer->buffer, sizeof(unsigned char), rle_writer->buffer_pos, rle_writer->file);
        if (result < rle_writer->buffer_pos) {
            fprintf(stderr, "\n[ERROR]: flush_writer() {} -> Unable to flush the buffer!\n");
            return -1;
        }
        rle_writer->buffer_pos = 0;
        rle_writer->counter_pos = -1;
    }
    return processed;
}

/*
* Function: flush_reader
* ----------------------
*  Flushes the remaining data in buffer to the output file.
*
*  rle_readaer: Pointer to the initiated RLEReader
*
*  returns: Written bytes count.
*/
int flush_reader(RLEReader* rle_reader) {
    if (rle_reader == NULL) {
        fprintf(stderr, "\n[ERROR]: flush_reader() {} -> RLEReader is NULL!\n");
        return -1;
    }

    size_t flushed_bytes = 0;
    
    if (rle_reader->buffer_pos > 0) {
        size_t result = fwrite(rle_reader->buffer, sizeof(unsigned char), rle_reader->buffer_pos, rle_reader->file);
        if (result < rle_reader->buffer_pos) {
            fprintf(stderr, "\n[ERROR]: flush_reader() {} -> Unable to flush the buffer!\n");
            return 0;
        }
        flushed_bytes = rle_reader->buffer_pos;
        rle_reader->buffer_pos = 0;
    }
    return flushed_bytes;
}

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
ssize_t encode(FILE* input_file, RLEWriter* rle_writer, size_t chunk_size) {
    if (input_file == NULL) {
        fprintf(stderr, "[ERROR]: encode() {} -> File pointer is NULL!\n");
        return -1;
    }

    unsigned char* read_buffer = malloc(chunk_size * sizeof(unsigned char));
    if (read_buffer == NULL) {
        fprintf(stderr, "\n[ERROR]: encode() {} -> Unable to allocate memory for buffer!\n");
        return -1;
    }

    size_t read_bytes = 0;
    size_t file_size = get_file_size(input_file);
    size_t processed = 0;
    fseek(input_file, 0, SEEK_SET);
    clock_t start_time = clock();

    unsigned char compression_mode_flag_byte = (unsigned char) rle_writer->compression_mode;
    if (fwrite(&compression_mode_flag_byte, sizeof(unsigned char), 1, rle_writer->file) < 1) {
        fprintf(stderr, "\n[ERROR]: encode() {} -> Unable to write the compression mode to the file!\n");
        free(read_buffer);
        return -1;
    }

    while ((read_bytes = fread(read_buffer, sizeof(unsigned char), chunk_size, input_file)) != 0) {
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

    if (rle_writer->buffer_pos > 0) {
        int result = flush_writer(rle_writer);
        if (result < 0) {
            free(read_buffer);
            return -1;
        }
    }

    clock_t end_time = clock();

    long compressed_file_size = ftell(rle_writer->file);
    int size_diff = file_size - compressed_file_size;
    double compression_rate = (double) abs(size_diff) / file_size * 100;
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("\rFinished processing (%f s): %zu bytes -> %ld bytes (%s%.2f%%)\n", time_spent, file_size, 
           compressed_file_size, size_diff > 0 ? "-" : "+", compression_rate);

    free(read_buffer);
    return processed;
}

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
ssize_t decode(FILE* input_file, RLEReader* rle_reader, size_t chunk_size) {
    if (input_file == NULL) {
        fprintf(stderr, "[ERROR]: decode() {} -> File pointer is NULL!\n");
        return -1;
    }

    unsigned char* read_buffer = malloc(chunk_size * sizeof(unsigned char));
    if (read_buffer == NULL) {
        fprintf(stderr, "\n[ERROR]: decode() {} -> Unable to allocate memory for buffer!\n");
        return -1;
    }

    size_t read_bytes = 0;
    size_t file_size = get_file_size(input_file);
    size_t processed = 0;
    clock_t start_time = clock();
    // Skip the first byte (compression mode byte)
    fseek(input_file, sizeof(unsigned char), SEEK_SET);

    while ((read_bytes = fread(read_buffer, sizeof(unsigned char), chunk_size, input_file)) != 0) {
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

    clock_t end_time = clock();
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    long compressed_file_size = ftell(rle_reader->file);
    printf("\rFinished Processing (%f s): %zu bytes -> %ld bytes\n", time_spent, file_size, compressed_file_size);

    free(read_buffer);
    return processed;
}

/*
* Function: print_buffer
* ----------------------
*  Prints HEX values of buffer content.
*
*  rle_writer: Pointer to the initiated RLEWriter.
*  cols: Number of Columns.
*/
void print_buffer(RLEWriter* rle_writer, int cols) {
    for (size_t i = 0; i < rle_writer->buffer_pos / cols + 1; i++) {
        for (int j = 0; j < cols && cols * i + j < rle_writer->buffer_pos; j++) {
            printf("%02X  ", rle_writer->buffer[cols*i+j]);
        }
        printf("\n");
    }
}
