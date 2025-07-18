#ifndef BITIO_H
#define BITIO_H
#include "constants.h"

#include <stdint.h>
#include <stdio.h>

typedef struct {
    unsigned char* buffer;
    FILE* file;
    size_t bit_count;
    size_t total_bits;
} BitWriter;

typedef struct {
    unsigned char* buffer;
    int bit_pos; // Bit position in current byte (0-7)
    int bit_padding;
    FILE* file;
    size_t buffer_pos; // Current byte position in buffer
    size_t buffer_size; // Bytes in buffer
    size_t bits_read; // Total bits read
    size_t total_bits; // Total valid bits BitReader;
} BitReader;

/*
* Function: init_writer
* ---------------------
*  Initiates a BitWriter object and sets the defaults.
*
*  file: A pointer to a file object
*
*  returns: A BitWriter object
*/
BitWriter* init_writer(FILE* file);

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
ssize_t write_bits(BitWriter* bit_writer, uint32_t code, uint8_t length);

/*
* Function: flush_writer
* ----------------------
*  Flushes the BitWriter buffer to the provided file in BitWriter object
*
*  bit_writer: Initiated BitWriter Object
*
*  returns: The number of written bytes. If failed, returns -1.
*/
ssize_t flush_writer(BitWriter* bit_writer);

/*
* Function: init_reader
* ---------------------
*  Initiates a BitReader object and sets the defaults.
*
*  file: A pointer to a file object
*
*  returns: A BitReader object. If failed, returns NULL.
*/
BitReader* init_reader(FILE* file);

/*
* Function: read_bits
* -------------------
*  returns the value of bit
*
*  bit_reader: Initiated BitReader object
*
*  returns: The value of the bit. if failed, returns -1.
*/
int read_bits(BitReader* bit_reader);
#endif
