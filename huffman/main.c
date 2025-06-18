#include "include/constants.h"
#include "include/utils.h"
#include "include/bitio.h"
#include "include/minheap.h"
#include "include/huffman.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int compress(FILE* input_file, FILE* output_file);
int decompress(FILE* input_file, FILE* output_file);

int main(int argc, char* argv[]) {
    int opt;
    int verbose_mode = 0;
    int compress_mode = 0;
    int decompress_mode = 0;
    int output_file_mode = 0;
    char* output_file_path = NULL;
    char* input_file_path = NULL;

    while ((opt = getopt(argc, argv, "c:d:o:v")) != -1) {
        switch (opt) {
            case 'c':
                if (decompress_mode) {
                    err("main", "Invalid flag combination!\n\tCan't use -c and -d at the same time.\n");
                    return EXIT_FAILURE;
                }
                compress_mode = 1;
                decompress_mode = 0;
                input_file_path = malloc(strlen(optarg));
                if (input_file_path == NULL) {
                    err("main", "Unable to allocate memory for input file name!\n");
                    return EXIT_FAILURE;
                }
                strcpy(input_file_path, optarg);
                break;
            case 'd':
                if (compress_mode) {
                    err("main", "Invalid flag combination!\n\tCan't use -c and -d at the same time.\n");
                    return EXIT_FAILURE;
                }
                decompress_mode = 1;
                compress_mode = 0;
                input_file_path = malloc(strlen(optarg));
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
                verbose_mode = 1;
                break;
            default:
                fprintf(stderr, "[USAGE]: %s [-c filename] [-d filename] [-o output_file_name] [-v]\n\t-c: compress file\n\t-d: decompress file\n\t-o: output file\n\t-v: print logs\n\r", argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (compress_mode && !decompress_mode) {
        if (!output_file_mode) {
            size_t output_file_size = strlen(input_file_path) + strlen(".huf") + 1;
            output_file_path = malloc(output_file_size);
            if (output_file_path == NULL) {
                err("main", "Unable to allocate memory for output file name!\n");
                return EXIT_FAILURE;
            }
            sprintf(output_file_path, "%s.huf", input_file_path);
            output_file_path[output_file_size - 1] = '\0';
        }

        FILE* input_file = open_file(input_file_path, "rb");
        FILE* output_file = open_file(output_file_path, "wb");

        if (input_file == NULL || output_file == NULL) {
            return EXIT_FAILURE;
        }

        int status = compress(input_file, output_file);
        fclose(input_file);
        fclose(output_file);
        printf("--->> Compression ");
        if (status) {
            printf("completed!\n");
        } else {
            printf("failed!\n");
            remove(output_file_path);
        }

    } else if (decompress_mode && !compress_mode) {
        if (!output_file_mode) {
            char* filename = NULL;
            char* file_extention = NULL;

            int status = extract_filename_format(input_file_path, &filename, &file_extention);
            if (status == -1) {
                err("main", "Invalid input file path!\n");
                return EXIT_FAILURE;
            } else if (status == 2 && (strcmp(strlwr(file_extention), "huf") == 0)) {
                size_t output_file_size = strlen(input_file_path) - strlen(".huf") + 1;
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

        int status = decompress(input_file, output_file);
        fclose(input_file);
        fclose(output_file);
        printf("--->> Decompression ");
        if (status) {
            printf("completed!\n");
        } else {
            printf("failed!\n");
            remove(output_file_path);
        }
    }

    free(output_file_path);
    free(input_file_path);
    return 0;
}

int compress(FILE* input_file, FILE* output_file) {
    size_t* frequencty_table = count_run(input_file);
    if (frequencty_table == NULL) {
        return 0;
    }

    Heap* priority_queue = create_priority_queue(frequencty_table);
    if (priority_queue == NULL) {
        free(frequencty_table);
        return 0;
    }
    
    Node* root = build_tree(priority_queue);
    if (root == NULL) {
        free(frequencty_table);
        free(priority_queue->nodes);
        free(priority_queue);
        return 0;
    }

    Code* code_table = malloc(FREQUENCY_TABLE_SIZE * sizeof(Code));
    if (code_table == NULL) {
        free(frequencty_table);
        free(priority_queue->nodes);
        free(priority_queue);
        free_tree(root);
        return 0;
    }
    
    generate_huffman_code(code_table, 0, 0, root);

    BitWriter* bit_writer = init_writer(output_file);
    if (bit_writer == NULL) {
        free(frequencty_table);
        free(priority_queue->nodes);
        free(priority_queue);
        free(code_table);
        free_tree(root);
        return 0;
    }

    size_t max_value = 0;
    size_t list_size = get_list_size(frequencty_table, &max_value);
    fwrite(&list_size, sizeof(unsigned char), 1, output_file);

    for (size_t i = 0; i < FREQUENCY_TABLE_SIZE; i++) {
        if (frequencty_table[i] > 0) {
            unsigned char symbol = (unsigned char) i;
            fwrite(&symbol, sizeof(unsigned char), 1, output_file);
            fwrite(&frequencty_table[i], sizeof(unsigned char), 1, output_file);
        }
    }

    int status = encode(input_file, bit_writer, code_table);
    if (status == 0) {
        free(frequencty_table);
        free(priority_queue->nodes);
        free(priority_queue);
        free(code_table);
        free(bit_writer);
        free_tree(root);
        return 0;
    }
    size_t total_bits = bit_writer->total_bits;
    printf("%zu ...\n", total_bits);
    fwrite(&total_bits, sizeof(size_t), 1, output_file);

    free(frequencty_table);
    free(priority_queue->nodes);
    free(priority_queue);
    free(code_table);
    free(bit_writer);
    free_tree(root);
    return 1;
}

int decompress(FILE* input_file, FILE* output_file) {
    BitReader* bit_reader = init_reader(input_file);
    if (bit_reader == NULL) {
        return 0;
    }

    size_t total_bits = 0;
    size_t* frequencty_table = read_file_header(input_file, &total_bits);
    if (frequencty_table == NULL) {
        free(bit_reader);
        return 0;
    }

    Heap* priority_queue = create_priority_queue(frequencty_table);
    if (priority_queue == NULL) {
        free(bit_reader);
        free(frequencty_table);
        return 0;
    }
    
    Node* root = build_tree(priority_queue);
    if (root == NULL) {
        free(bit_reader);
        free(frequencty_table);
        free(priority_queue->nodes);
        free(priority_queue);
        return 0;
    }

    int status = decode(output_file, bit_reader, root, total_bits);

    free(bit_reader);
    free(frequencty_table);
    free(priority_queue->nodes);
    free(priority_queue);
    return status;
}
