#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>

#define BUFFER_SIZE 256
#define DICTIONARY_SIZE 32 * 1024
#define OUTPUT_CHUNK_SIZE 4 * 1024

typedef struct {
    unsigned char* dictionary;
    unsigned char* buffer;
} Window;

size_t file_size = 0;
int DEBUG = 0;

size_t strlength(const char* str);
int strcomp(const char* s1, const char* s2);
void memoryset(unsigned char* ptr, unsigned char value, size_t count);

void shift_buffer(unsigned char* buffer, size_t buffer_size, char chr, size_t shift);
size_t dict_push(unsigned char* dict, char chr, size_t* dict_last_pos, const size_t dict_size);

size_t get_file_size(FILE* file);

void print_usage(const char *prog_name);
int compress(FILE* original_file, FILE* compressed_file);
int decompress(FILE* compressed_file, FILE* original_file);

int main(int argc, char* argv[]) {
    if (argc >= 3) {
        // Compression
        if (strcomp(argv[1], "-c") == 0) {
            FILE* original_file = fopen(argv[2], "rb");
            if (original_file == NULL) {
                fprintf(stderr, "[ERROR]: main() {} -> Unable to open the file!\n");
                return 1;
            }

            // Calculating the file size
            file_size = get_file_size(original_file); 

            // Managing new filename
            char* compressed_filename = malloc(strlength(argv[2]) + 5);
            if (compressed_filename == NULL) {
                fprintf(stderr, "[ERROR]: main() {} -> Unable to allocate memory for comressed file name!\n");
                return 1;
            }
            sprintf(compressed_filename, "%s.lz7", argv[2]);

            // Creating a new compressed file
            FILE* compressed_file = fopen(compressed_filename, "wb");
            if (compressed_file == NULL) {
                fprintf(stderr, "[ERROR]: main() {} -> Unable to create compressed file!\n");
                return 1;
            }

            int status = compress(original_file, compressed_file);
            if (status > -1) {
                printf("Compressed successfully!\n");
            } else {
                printf("Compressed failed!\n");
            }

            fclose(original_file);
            fclose(compressed_file);

            free(compressed_filename);
        }
        else if (strcomp(argv[1], "-d") == 0) {
            FILE* compressed_file = fopen(argv[2], "rb");
            if (compressed_file == NULL) {
                fprintf(stderr, "[ERROR]: main() {} -> Unable to open the file!\n");
                return 1;
            }


            // Calculating the file size
            file_size = get_file_size(compressed_file);

            // Managing the original file name
            char* original_filename = malloc(strlength(argv[2]) - 4);
            if (original_filename == NULL) {
                fprintf(stderr, "[ERROR]: main() {} -> Unable to allocate memory for original file name!\n");
                return 1;
            }
            argv[2][strlength(argv[2]) - 4] = '\0';
            sprintf(original_filename, "%s", argv[2]);
            original_filename[strlength(argv[2])] = '\0';

            FILE* original_file = fopen(original_filename, "wb");
            if (original_file == NULL) {
                fprintf(stderr, "[ERROR]: main() {} -> Unable to create original file!\n");
                return 1;
            }

            int status = decompress(compressed_file, original_file);
            if (status > -1) {
                printf("Decompressed successfully!\n");
            } else {
                printf("Decompressed failed!\n");
            }

            fclose(original_file);
            fclose(compressed_file);

            free(original_filename);
        }
    }
    return EXIT_SUCCESS;
}

void shift_buffer(unsigned char* buffer, size_t buffer_size, char chr, size_t shift) {
    for (size_t i = 0; i + shift < buffer_size; i++) {
        buffer[i] = buffer[i + shift];
    }
    buffer[buffer_size - shift] = chr;

    for (size_t i = buffer_size - shift + 1; i < buffer_size; i++) {
        buffer[i] = 0;
    }
}

int strcomp(const char* s1, const char* s2) {
    unsigned char c1, c2;
    do {
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;

      if (c1 == '\0') 
            return c1 - c2;

    } while (c1 == c2);

  return c1 - c2;
}

size_t dict_push(unsigned char* dict, char chr, size_t* dict_last_pos, const size_t dict_size) {
    dict[*dict_last_pos] = chr;
    *dict_last_pos = (*dict_last_pos + 1) % dict_size;
    return 1;
}

int compress(FILE* original_file, FILE* compressed_file) {
    // Declaring buffers
    Window slider;
    slider.buffer = malloc(BUFFER_SIZE);
    slider.dictionary = malloc(DICTIONARY_SIZE);
    unsigned char* output_buffer = malloc(3 * BUFFER_SIZE);
    if (slider.dictionary == NULL || slider.buffer == NULL || output_buffer == NULL) {
        fprintf(stderr, "[ERROR]: main() {} -> Unable to allocate memory for dictionary and buffer!\n");
        free(slider.buffer);
        free(slider.dictionary);
        free(output_buffer);
        return -1;
    }
    memoryset(slider.dictionary, 0, DICTIONARY_SIZE);

    size_t bytes_read = 0;
    size_t output_pos = 0;
    size_t dict_size = 0;
    size_t process = 0;

    while ((bytes_read = fread(slider.buffer, 1, BUFFER_SIZE, original_file)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            if (DEBUG) printf("[%04lld] (%2X):\t", process + i, slider.buffer[i]);

            unsigned char current_char = slider.buffer[i];
            size_t best_match_pos = dict_size;
            uint8_t best_match_length = 0;

            for (size_t j = 0; j < dict_size; j++) {
                if (current_char == slider.dictionary[j]) {
                    size_t match_length = 0;
                    for (size_t k = j; k < dict_size && i + match_length < bytes_read - 1; k++) {
                        if (slider.buffer[i + match_length] == slider.dictionary[k]) {
                            match_length++;
                        } else {
                            break;
                        }
                    }

                    if (match_length >= best_match_length) {
                        best_match_length = match_length;
                        best_match_pos = j;
                    }
                } 
            }
            
            if (best_match_length) {
                uint16_t offset = dict_size - best_match_pos;
                uint8_t length = best_match_length;

                if (DEBUG) printf("MATCH\t <%04d, %04d>\t\t-> |", offset, length);

                for (size_t l = 0; l < best_match_length; l++) {
                    dict_push(slider.dictionary, slider.buffer[i + l], &dict_size, DICTIONARY_SIZE);
                    if (DEBUG) printf("%2X|", slider.buffer[i + l]);
                }

                output_buffer[output_pos++] = (offset >> 8) & 0xFF; // Write high byte
                output_buffer[output_pos++] = offset & 0xFF;        // Write low byte
                output_buffer[output_pos++] = length;               // Write low byte

                i += best_match_length - 1;
            } else {
                output_buffer[output_pos++] = 0;
                output_buffer[output_pos++] = 0;
                output_buffer[output_pos++] = slider.buffer[i];

                if (DEBUG) printf("LITERAL\t <0000, 0000, %2X>", slider.buffer[i]);

                dict_push(slider.dictionary, current_char, &dict_size, DICTIONARY_SIZE);
            }

            if (output_pos + 2 >= BUFFER_SIZE) {
                if (fwrite(output_buffer, sizeof(unsigned char), output_pos, compressed_file) != output_pos) {
                    fprintf(stderr, "[ERROR]: compress() {} -> Unable to flush output to file!\n");
                    free(output_buffer);
                    free(slider.buffer);
                    free(slider.dictionary);
                    return -1;
                }
                output_pos = 0;
            }

            if (DEBUG) printf("\n");
        }

        process += bytes_read;
        if ( process % (100 * 256) == 0) {
            if (!DEBUG) printf("\rProcessing: %lld/%lld -> %ld", process, file_size, ftell(compressed_file));
        }
    }

	if (!DEBUG) printf("\rProcessing: %lld/%lld -> %ld\n", process, file_size, ftell(compressed_file));

    if (output_pos > 0) {
        if (fwrite(output_buffer, sizeof(unsigned char), output_pos, compressed_file) != output_pos) {
            fprintf(stderr, "[ERROR]: compress() {} -> Unable to flush output to file!\n");
            free(output_buffer);
            free(slider.buffer);
            free(slider.dictionary);
            return -1;
        }
    }

    free(slider.dictionary);
    free(slider.buffer);
    free(output_buffer);
    return 0;
}

int decompress(FILE* compressed_file, FILE* original_file) {
    unsigned char* output_buffer = malloc(OUTPUT_CHUNK_SIZE);
    unsigned char* dictionary = malloc(DICTIONARY_SIZE);
    unsigned char* read_buffer = malloc(BUFFER_SIZE * 3);
    if (output_buffer == NULL || read_buffer == NULL || dictionary == NULL) {
        fprintf(stderr, "[ERROR]: main() {} -> Unable to allocate memory for buffer!\n");
        free(output_buffer);
        free(dictionary);
        free(read_buffer);
        return -1;
    }
    memoryset(dictionary, 0, DICTIONARY_SIZE);
    memoryset(output_buffer, 0, OUTPUT_CHUNK_SIZE);

    size_t bytes_read = 0;
    size_t dict_pos = 0;
    size_t output_pos = 0;
    size_t process = 0;

    while ((bytes_read = fread(read_buffer, sizeof(unsigned char), BUFFER_SIZE * 3, compressed_file)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            uint16_t offset = ((uint16_t) read_buffer[i] << 8) | read_buffer[i + 1]; // Read 2 bytes
            if (offset) {
                if ( i + 4 >= bytes_read) {
                    fseek(compressed_file, i - bytes_read, SEEK_CUR);
                    break;
                }

                if (DEBUG) printf("[%04lld] <%03d, %03d>\t[%2X][%2X], %lld\t\t -> |", process + i, offset, read_buffer[i + 2], read_buffer[i], read_buffer[i + 1], output_pos);

                offset = dict_pos - offset;
                i += 2;
                uint8_t length = read_buffer[i];

                for (uint8_t j = 0; j < length; j++) {
                    if (DEBUG) printf("%2X|", output_buffer[offset + j]);

                    output_buffer[output_pos++] = dictionary[offset + j];
                    dict_push(dictionary, dictionary[offset + j], &dict_pos, DICTIONARY_SIZE); 

                    if (output_pos >= OUTPUT_CHUNK_SIZE) {
                    // Flush output buffer to file
                        if (fwrite(output_buffer, 1, output_pos, original_file) != output_pos) {
                            fprintf(stderr, "[ERROR]: decompress() {} -> Unable to flush output to file!\n");
                            free(dictionary);
                            free(read_buffer);
                            free(output_buffer);
                            return -1;
                        }
                        output_pos = 0;
                    }
                }
            } else {
                if (DEBUG) printf("[%04lld] <000, 000, %2X>", process + i, read_buffer[i + 2]);

                i += 2;
                output_buffer[output_pos++] = read_buffer[i];
                dict_push(dictionary, read_buffer[i], &dict_pos, DICTIONARY_SIZE); 

                if (output_pos >= OUTPUT_CHUNK_SIZE) {
                    // Flush output buffer to file
                    if (fwrite(output_buffer, 1, output_pos, original_file) != output_pos) {
                        fprintf(stderr, "[ERROR]: decompress() {} -> Unable to flush output to file!\n");
                        free(dictionary);
                        free(read_buffer);
                        free(output_buffer);
                        return -1;
                    }
                    output_pos = 0;
                }
            }

            if (DEBUG) printf("\n");
        }
        
        process += bytes_read;
        if ( process % (100 * 1024) == 0) {
            if (!DEBUG) printf("\rProcessing: %lld/%lld -> %ld", process, file_size, ftell(original_file));
        }
    }

    if (output_pos > 0) {
        // Flush output buffer to file
        if (fwrite(output_buffer, 1, output_pos, original_file) != output_pos) {
            fprintf(stderr, "[ERROR]: decompress() {} -> Unable to flush output to file!\n");
            free(dictionary);
            free(read_buffer);
            free(output_buffer);
            return -1;
        }
    }

	if (!DEBUG) printf("\rProcessing: %lld/%lld -> %ld\n", process, file_size, ftell(original_file));

    free(output_buffer);
    free(read_buffer);
    free(dictionary);
    return 1;
}

size_t strlength(const char* str) {
    if (str == NULL) return 0;

    size_t length = 0;
    while (*str != '\0') {
        str++;
        length++;
    }

    return length;
}

void memoryset(unsigned char* ptr, unsigned char value, size_t count) {
    while (count-- > 0) {
        *ptr++ = value;
    }
}

size_t get_file_size(FILE* file) {
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [-c input output | -d input output] [-v]\n", prog_name);
    fprintf(stderr, "  -c: Compress input file to output file\n");
    fprintf(stderr, "  -d: Decompress input file to output file (example: file.lz7)\n");
    fprintf(stderr, "  -o: Specify output file\n");
    fprintf(stderr, "  -v: Enable verbose logging\n");
    exit(EXIT_FAILURE);
}
