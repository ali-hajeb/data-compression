#include "include/constants.h"
#include "include/compressor.h"
#include "include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    int opt;
    int compress_mode = 0;
    int decompress_mode = 0;
    int output_file_mode = 0;
    // int verbose_mode = 0;
    char* output_file_path = NULL;
    char* input_file_path = NULL;
    size_t compressed_buffer_size = COMPRESSED_BUFFER_SIZE;
    size_t decompressed_buffer_size = DECOMPRESSED_BUFFER_SIZE;
    size_t window_size = WINDOW_SIZE;

    // Setting up the CLI
    while ((opt = getopt(argc, argv, "c:d:o:w:B:b:v")) != -1) {
        switch (opt) {
            case 'c':
                if (decompress_mode) {
                    err("main", "Invalid flag combination!"
                                "\n\tCan't use -c and -d at the same time.\n");
                    return EXIT_FAILURE;
                }
                compress_mode = 1;
                decompress_mode = 0;
                input_file_path = malloc(strlen(optarg) + 1);
                if (input_file_path == NULL) {
                    err("main", "Unable to allocate memory for input file name!\n");
                    return EXIT_FAILURE;
                }
                strcpy(input_file_path, optarg);
                break;
            case 'd':
                if (compress_mode) {
                    err("main", "Invalid flag combination!"
                                "\n\tCan't use -c and -d at the same time.\n");
                    return EXIT_FAILURE;
                }
                decompress_mode = 1;
                compress_mode = 0;
                input_file_path = malloc(strlen(optarg) + 1);
                if (input_file_path == NULL) {
                    err("main", "Unable to allocate memory for input file name!\n");
                    return EXIT_FAILURE;
                }
                strcpy(input_file_path, optarg);
                break;
            case 'o':
                output_file_mode = 1;
                output_file_path = malloc(strlen(optarg) + 1);
                if (output_file_path == NULL) {
                    err("main", "Unable to allocate memory for output file name!\n");
                    return EXIT_FAILURE;
                }
                strcpy(output_file_path, optarg);
                break;
            case 'v':
                // verbose_mode = 1;
                break;
            case 'w': {
                size_t w_size = 0;
                if (scanf(optarg, "%zu", &w_size) == 1) {
                    window_size = w_size;
                }
                break;
            }
            case 'b': {
                size_t c_buffer_size = 0;
                if (scanf(optarg, "%zu", &c_buffer_size) == 1) {
                    compressed_buffer_size = c_buffer_size;
                }
                break;
            }
            case 'B': {
                size_t d_buffer_size = 0;
                if (scanf(optarg, "%zu", &d_buffer_size) == 1) {
                    decompressed_buffer_size = d_buffer_size;
                }
                break;
            }
            default:
                fprintf(stderr, "[USAGE]: %s [-c filename] [-d filename] [-o output_file_name] [-v]"
                                "\n\t-c: compress file"
                                "\n\t-d: decompress file"
                                "\n\t-o: output file"
                                "\n\t-w: window slider (dictionary) size (default: %d bytes)"
                                "\n\t-b: compressed buffer (reader/writer buffer) size (default: %d bytes)"
                                "\n\t-B: decompressed buffer (chunck reader) size (default: %d bytes)"
                                "\n\t-v: print logs\n\r", 
                                argv[0], (WINDOW_SIZE), (COMPRESSED_BUFFER_SIZE), (DECOMPRESSED_BUFFER_SIZE));
                return EXIT_FAILURE;
        }
    }

    // Compression mode:
    if (compress_mode && !decompress_mode) {
        // If user did not specify an output path, add '.lz7' at the end of the input file
        if (!output_file_mode) {
            size_t output_file_size = strlen(input_file_path) + strlen(".lz7") + 1;
            output_file_path = malloc(output_file_size);
            if (output_file_path == NULL) {
                err("main", "Unable to allocate memory for output file name!\n");
                return EXIT_FAILURE;
            }
            sprintf(output_file_path, "%s.lz7", input_file_path);
            output_file_path[output_file_size - 1] = '\0';
        }

        FILE* input_file = open_file(input_file_path, "rb");
        FILE* output_file = open_file(output_file_path, "wb");

        if (input_file == NULL || output_file == NULL) {
            return EXIT_FAILURE;
        }

        int result = compress(input_file, output_file, compressed_buffer_size, decompressed_buffer_size, window_size);
        fclose(input_file);
        fclose(output_file);
        printf("\n\t--->> Compression ");
        if (result) {
            printf("completed!\n");
        } else {
            printf("failed!\n");
            remove(output_file_path);
        }

    } 
    // Decompression mode
    else if (decompress_mode && !compress_mode) {
        // If user did not specify an output path:
        //  - If file has .lz7 at the end, remove it
        //  - Or use the same path as input
        if (!output_file_mode) {
            char* filename = NULL;
            char* file_extention = NULL;

            int result = extract_filename_format(input_file_path, &filename, &file_extention);
            if (result == -1) {
                err("main", "Invalid input file path!\n");
                return EXIT_FAILURE;
            } else if (result == 2 && (strcasecmp(file_extention, "lz7") == 0)) {
                size_t output_file_size = strlen(input_file_path) - strlen(".lz7") + 1;
                output_file_path = malloc(output_file_size);
                if (output_file_path == NULL) {
                    err("main", "Unable to allocate memory for output file name!\n");
                    return EXIT_FAILURE;
                }
                strncpy(output_file_path, input_file_path, output_file_size - 1);
                output_file_path[output_file_size - 1] = '\0';
            } else {
                size_t output_file_size = strlen(input_file_path) + 1;
                output_file_path = malloc(output_file_size);
                if (output_file_path == NULL) {
                    err("main", "Unable to allocate memory for output file name!\n");
                    return EXIT_FAILURE;
                }
                strcpy(output_file_path, input_file_path);
            }
        }

        FILE* input_file = open_file(input_file_path, "rb");
        FILE* output_file = open_file(output_file_path, "wb");

        if (input_file == NULL || output_file == NULL) {
            return EXIT_FAILURE;
        }

        int result = decompress(input_file, output_file, compressed_buffer_size, decompressed_buffer_size, window_size);
        fclose(input_file);
        fclose(output_file);
        printf("\n\t--->> Decompression ");
        if (result) {
            printf("completed!\n");
        } else {
            printf("failed!\n");
            remove(output_file_path);
        }
    }

    printf("\n\r");
    free(output_file_path);
    free(input_file_path);
    return 0;
}
