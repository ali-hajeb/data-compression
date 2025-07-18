#include "../include/constants.h"
#include "../include/bitio.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
* Function: init_writer
* ---------------------
*  Initiates a BitWriter object and sets the defaults.
*
*  file: A pointer to a file object
*
*  returns: A BitWriter object. If failed, returns NULL.
*/
BitWriter* init_writer(FILE* file) {
    if (file == NULL) {
        fprintf(stderr, "\n[ERROR]: init_writer() {} -> File does not exit!\n");
        return NULL;
    }
    BitWriter* bit_writer = malloc(sizeof(BitWriter));
    if (bit_writer == NULL) {
        fprintf(stderr, "\n[ERROR]: init_writer() {} -> Unable to allocate memory for bit_writer!\n");
        return NULL;
    }
    bit_writer->file = file;
    bit_writer->bit_count = 0;
    bit_writer->total_bits = 0;
    bit_writer->buffer = malloc(OUTPUT_BUFFER_SIZE * sizeof(unsigned char));
    if (bit_writer->buffer == NULL) {
        fprintf(stderr, "\n[ERROR]: init_writer() {} -> Unable to allocate memory for bit_writer->buffer!\n");
        return NULL;
    }
    // memset(bit_writer->buffer, 0, OUTPUT_BUFFER_SIZE);
    return bit_writer;

}

/*
* Function: write_bits
* --------------------
*  Writes the given bits to the BitWriter.
*
*  bit_writer: Initiated BitWriter object
*  code: The bit code to be written
*  length: bit counts
*
*  returns: The number of written bits. If failed, returns -1.
*/
ssize_t write_bits(BitWriter* bit_writer, uint32_t code, uint8_t length) {
    if (bit_writer == NULL) {
        fprintf(stderr, "\n[ERROR]: write_bits() {} -> Bit writer is NULL!\n");
        return -1;
    }
    // if no bits, return 0
    if (length == 0) return 0;

    for (uint8_t i = 0; i < length; i++) {
        // Find the current byte index
        size_t byte_idx = bit_writer->bit_count / 8;
        // Find the current bit index
        size_t bit_idx = bit_writer->bit_count % 8;

        // Flush writer buffer
        if (byte_idx >= OUTPUT_BUFFER_SIZE) {
            size_t written_bytes = fwrite(bit_writer->buffer, 1, OUTPUT_BUFFER_SIZE, bit_writer->file);
            if (written_bytes < OUTPUT_BUFFER_SIZE) {
                fprintf(stderr, "\n[ERROR]: init_bitwriter() {} -> Unable to flush the bit_writer!\n");
                return -1;
            }

            // Reset the buffer and indexes
            memset(bit_writer->buffer, 0, OUTPUT_BUFFER_SIZE);
            bit_writer->bit_count = 0;
            byte_idx = 0;
            bit_idx = 0;
        }

        // Extract bit from code (MSB first)
        int bit = (code >> (length - 1 - i)) & 1;
        // Set bit in buffer
        bit_writer->buffer[byte_idx] |= (bit << (7 - bit_idx));
        bit_writer->bit_count++;
        bit_writer->total_bits++;
    }

    return length;
}

/*
* Function: flush_writer
* ----------------------
*  Flushes the BitWriter buffer to the provided file in BitWriter object
*
*  bit_writer: Initiated BitWriter Object
*
*  returns: The number of written bytes. If failed, returns -1.
*/
ssize_t flush_writer(BitWriter* bit_writer) {
    if (bit_writer == NULL) {
        fprintf(stderr, "\n[ERROR]: flush_writer() {} -> Bit writer is NULL!\n");
        return -1;
    }
    // Calculate the maximum required bytes
    size_t bytes = (bit_writer->bit_count + 7) / 8;
    size_t written_bytes = 0;
    if (bytes > 0) {
        written_bytes = fwrite(bit_writer->buffer, 1, bytes, bit_writer->file);
        if (written_bytes == 0) {
            fprintf(stderr, "\n[ERROR]: flush_writer() {} -> Unable to flush the bit_writer!\n");
            return -1;
        }
    }
    // Reset bit_writer
    memset(bit_writer->buffer, 0, OUTPUT_BUFFER_SIZE);
    bit_writer->bit_count = 0;
    return written_bytes;
}

/*
* Function: init_reader
* ---------------------
*  Initiates a BitReader object and sets the defaults.
*
*  file: A pointer to the input file object
*
*  returns: A BitReader object. If failed, returns NULL.
*/
BitReader* init_reader(FILE* file) {
    if (file == NULL) {
        fprintf(stderr, "\n[ERROR]: init_reader() {} -> File does not exit!\n");
        return NULL;
    }
    BitReader* bit_reader = malloc(sizeof(BitReader));
    if (bit_reader == NULL) {
        fprintf(stderr, "\n[ERROR]: init_reader() {} -> Unable to allocate memory for bit_reader!\n");
        return NULL;
    }
    bit_reader->file = file;
    bit_reader->bit_padding = 0;
    bit_reader->bit_pos = 8;
    bit_reader->bits_read = 0;
    bit_reader->buffer_pos = 0;
    bit_reader->buffer_size = 0;
    bit_reader->buffer = malloc(READ_BUFFER_SIZE * sizeof(unsigned char));
    if (bit_reader->buffer == NULL) {
        fprintf(stderr, "\n[ERROR]: init_reader() {} -> Unable to allocate memory for bit_reader->buffer!\n");
        return NULL;
    }
    // memset(bit_reader->buffer, 0, READ_BUFFER_SIZE);
    return bit_reader;
}

/*
* Function: read_bits
* -------------------
*  returns the value of bit
*
*  bit_reader: Initiated BitReader object
*
*  returns: The value of the bit. if failed, returns -1.
*/
int read_bits(BitReader* bit_reader) {
    if (bit_reader == NULL) {
        fprintf(stderr, "\n[ERROR]: read_bits() {} -> Bit reader is NULL!\n");
        return -1;
    }
    // Reset read_buffer
    if (bit_reader->bit_pos == 8) {
        // Read new data
        if (bit_reader->buffer_pos >= bit_reader->buffer_size) {
            bit_reader->buffer_size = fread(bit_reader->buffer, 1, READ_BUFFER_SIZE, bit_reader->file);
            if (bit_reader->buffer_size == 0) {
                return -1;
            }
            bit_reader->buffer_pos = 0;
        }
        bit_reader->bit_pos = 0;
        bit_reader->buffer_pos++;
    }
    int bit = (bit_reader->buffer[bit_reader->buffer_pos - 1] >> (7 - bit_reader->bit_pos)) & 1;
    bit_reader->bit_pos++;
    bit_reader->bits_read++;
    return bit;
}
