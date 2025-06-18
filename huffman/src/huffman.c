#include "../include/constants.h"
#include "../include/utils.h"
#include "../include/bitio.h"
#include "../include/huffman.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
* Function: count_run
* -------------------
*  Calculates the occurance of every character
*
*  file: Pointer to the input file
*
*  returns: Array of frequencies
*/
size_t* count_run(FILE* file) {
    size_t* frequency_table = malloc(FREQUENCY_TABLE_SIZE * sizeof(size_t));
    if (frequency_table == NULL) {
        fprintf(stderr, "\n[ERROR]: count_run() {} -> Unable to allocate memory for frequency table!\n");
        return NULL;
    }
    // set every value to zero, in order to start counting occurance
    memset(frequency_table, 0, FREQUENCY_TABLE_SIZE * sizeof(size_t)); 

    char* read_buffer = malloc(READ_BUFFER_SIZE * sizeof(unsigned char));
    if (read_buffer == NULL) {
        fprintf(stderr, "\n[ERROR]: count_run() {} -> Unable to allocate memory for read buffer!\n");
        free(frequency_table);
        return NULL;
    }

    size_t read_bytes = 0;
    while ( (read_bytes = fread(read_buffer, sizeof(unsigned char), READ_BUFFER_SIZE, file)) != 0) {
        for (size_t i = 0; i < read_bytes; i++) {
            frequency_table[(unsigned char) read_buffer[i]]++;
        }
    }

    free(read_buffer);
    return frequency_table;
}

/*
* Function: read_file_header
* --------------------------
*  Reads the header information of compressed file.
*
*  input_file: Pointer to the compressed file
*  bit_count: Pointer to the variable storing total bit count
*
*  returns: Frequency table
*/
size_t* read_file_header(FILE* input_file, size_t* bit_count) {
    size_t* frequency_table = malloc(FREQUENCY_TABLE_SIZE * sizeof(size_t));
    if (frequency_table == NULL) {
        fprintf(stderr, "\n[ERROR]: count_run() {} -> Unable to allocate memory for frequency table!\n");
        return NULL;
    }
    // set every value to zero, in order to start counting occurance
    memset(frequency_table, 0, FREQUENCY_TABLE_SIZE * sizeof(size_t)); 

    size_t char_count = 0;
    fseek(input_file, 0, SEEK_SET);
    size_t read_bytes = fread(&char_count, 1, 1, input_file);
    if (read_bytes <= 0) {
        fprintf(stderr, "\n[ERROR]: decompress() {} -> File is corrupted! %zu\n", char_count);
        return NULL;
    }

    unsigned char* read_buffer = malloc(char_count * sizeof(unsigned char) * 2);
    if (read_buffer == NULL) {
        fprintf(stderr, "\n[ERROR]: decompress() {} -> Unable to allocate memory for buffer!\n");
        return NULL;
    }
    read_bytes = fread(read_buffer, sizeof(unsigned char), char_count * 2, input_file);
    if (read_bytes <= 0) {
        fprintf(stderr, "\n[ERROR]: decompress() {} -> File is corrupted!\n");
        return NULL;
    }

    for (int i = 0; i < char_count * 2; i += 2) {
        if (i >= char_count * 2) printf("\n");
        unsigned char symbol = read_buffer[i];
        unsigned char count = read_buffer[i + 1];
        frequency_table[symbol] = count;
    }

    free(read_buffer);
    size_t header_end_pos = ftell(input_file);
    long total_bits_pos = sizeof(size_t);
    fseek(input_file, -1 * total_bits_pos, SEEK_END);
    fread(bit_count, sizeof(size_t), 1, input_file);
    fseek(input_file, header_end_pos, SEEK_SET);

    return frequency_table;
}

/*
* Function: generate_huffman_code
* -------------------------------
*  Generates binary codes based on the huffman tree for every leaf node
*
*  code_tabel: Array of codes to store the data
*  code: huffman binary code
*  depth: (default 0)
*  node: root node of the tree
*/
void generate_huffman_code(Code* code_table, uint32_t code, uint8_t depth, Node* node) {
    if (node == NULL) return;
    if (node->l_node == NULL && node->r_node == NULL) {
        code_table[node->symbol].code = code; 
        code_table[node->symbol].length = depth; 
        return;
    }

    if (node->r_node != NULL) {
        generate_huffman_code(code_table, (code << 1) | 1, depth + 1, node->r_node);
    }
    if (node->l_node != NULL) {
        generate_huffman_code(code_table, (code << 1), depth + 1, node->l_node);
    }
}

/*
* Function: encode
* ----------------
*  Encodes the input file using huffman encoding and saves the output to a file using BitWriter
*
*  input_file: Pointer to the file to be encoded
*  bit_writer: Pointer to the BitWriter object
*  code_table: Pointer to the code table
*
*  returns: If failed (0), On success (1)
*/
int encode(FILE* input_file, BitWriter* bit_writer, Code* code_table) {
    unsigned char read_buffer[READ_BUFFER_SIZE];
    size_t bytes_read = 0;
    size_t file_size = get_file_size(input_file);
    size_t processed = 0;
    fseek(input_file, 0, SEEK_SET);

    while((bytes_read = fread(read_buffer, 1, READ_BUFFER_SIZE, input_file)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            unsigned char symbol = read_buffer[i];
            if (code_table[symbol].length > 0) {
                int status = write_bits(bit_writer, code_table[symbol].code, code_table[symbol].length);
                if (status == -1) {
                    return 0;
                }
            }
        }
        processed += bytes_read;
        if (processed % (100 * KB)) {
            printf("\rProcessing: %lld/%lld bytes. -> %ld bytes.", processed, file_size, ftell(bit_writer->file));
        }
    }

    int status = flush_writer(bit_writer);
    if (status == -1) {
        return 0;
    }

    printf("\rProcessing: %lld/%lld bytes. -> %ld bytes.\n", processed, file_size, ftell(bit_writer->file));
    return 1;
}

/*
* Function: decode
* ----------------
*  Decodes a huffman file and save it to the output file.
*
*  output_file: Pointer to the output file.
*  bit_reader: Pointer to a BitReader object.
*  root: Pointer to the root node of the huffman tree.
*  total_bits: Number of encoded bits.
*
*  returns: If failed (0), on success (1)
*/
int decode(FILE *output_file, BitReader *bit_reader, Node* root, size_t total_bits) {
    unsigned char out_buffer[OUTPUT_BUFFER_SIZE];
    size_t out_pos = 0;
    size_t file_size = get_file_size(bit_reader->file);
    size_t processed = ftell(bit_reader->file);
    Node* current = root;

    printf("%zu\n", total_bits);
    while (bit_reader->bits_read < total_bits) {
        int bit = read_bits(bit_reader);
        if (bit == -1) {
            break;
        }
        current = bit ? current->r_node : current->l_node;
        // Leaf Node:
        if (current->l_node == NULL && current->r_node == NULL) { 
            out_buffer[out_pos++] = current->symbol;
            // Flush output_buffer
            if (out_pos == OUTPUT_BUFFER_SIZE) {
                size_t written_bytes = fwrite(out_buffer, 1, OUTPUT_BUFFER_SIZE, output_file);
                if (written_bytes <= 0) {
                    return 0;
                }
                out_pos = 0;
            }
            current = root;
        }
        processed += bit_reader->bits_read;
        if ((processed / 8) % (100 * KB)) {
            printf("\rProcessing: %lld/%lld bytes. -> %ld bytes.", processed / 8, file_size, ftell(output_file));
        }
    }

    if (out_pos > 0) {
        size_t written_bytes = fwrite(out_buffer, 1, out_pos, output_file);
        if (written_bytes <= 0) {
            return 0;
        }
    }
    printf("\rProcessing: %lld/%lld bytes. -> %ld bytes.\n", processed / 8, file_size, ftell(output_file));

    return 1;
}
