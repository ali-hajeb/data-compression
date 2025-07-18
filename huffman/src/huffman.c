#include "../include/constants.h"
#include "../include/utils.h"
#include "../include/bitio.h"
#include "../include/huffman.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
* Function: get_list_size
* -----------------------
*  Returns the number of non-zero values. Also can obtain the maximum value.
*
*  list: Pointer to the list array.
*  max_value: Pointer to the max_value variable.
*
*  returns: Count of non-zero values.
*/
size_t get_list_size(size_t* list, size_t* max_value) {
    size_t list_size = 0;
    for (size_t i = 0; i < FREQUENCY_TABLE_SIZE; i++) {
        if (list[i] != 0) {
            list_size++;
            if (list[i] > *max_value) {
                *max_value = list[i];
            }
        }
    }
    *max_value /= 255;
    *max_value += 1;
    return list_size;
}

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
* Function compare_nodes
* ----------------------
*  Compares the frequency of two nodes
*
*  a: Pointer to Node 1
*  b: Pointer to Node 2
*
*  returns: The difference of the nodes priority
*/
int compare_nodes(const void* a, const void* b) {
    const Node* node_a = (Node*) a;
    const Node* node_b = (Node*) b;

    if (node_a->frequency == node_b->frequency) {
        return node_a->symbol - node_b->symbol;
    } else {
        return node_a->frequency - node_b->frequency;
    }
}
/*
* Function combine_nodes
* ----------------------
*  Combines the nodes and returns a new node.
*
*  n1: First node.
*  n2: Second node.
*
*  returns: New node with 2 childeren
*/
Node* combine_nodes(Node* n1, Node* n2) {
    Node* new_node = malloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "\n[ERROR]: combine_nodes() {} -> Unable to combine two nodes!\n");
        return NULL;
    }

    new_node->symbol = 0xFF;
    new_node->frequency = n1->frequency + n2->frequency;
    new_node->l_node = n1;
    new_node->r_node = n2;
    return new_node;
}

/*
* Function build_tree.
* -------------------
*  Builds the binary tree of the min-heap.
*
*  heap: Pointer to the min-heap.
*
*  returns: A pointer to the root of the tree.
*/
Node* build_tree(Heap* heap) {
    while (heap->size >= 2) {
        Node* n1 = (Node*) heap_extract(heap);
        Node* n2 = (Node*) heap_extract(heap);
        if (n1 == NULL || n2 == NULL) {
            return NULL;
        }

        Node* new_node = combine_nodes(n1, n2);
        if (new_node == NULL) {
            return NULL;
        }

        ssize_t index = heap_insert(heap, new_node);
        if (index == -1) {
            free(new_node);
            return NULL;
        }
    }

    return heap_extract(heap);
}

/*
* Function: print_tree
* --------------------
*  Prints the tree (Recursively).
*
*  root: Pointer to the root of the tree
*  indent: Number of space indentation after each branch
*/
void print_tree(Node* root, int indent) {
    printf("%*s[%02X (%c): (%zu)] ->\n", indent, " ", root->symbol, root->symbol, root->frequency);

    if (root->r_node == NULL && root->l_node == NULL) {
        return;
    } 

    indent += 5;
    print_tree(root->r_node, indent);
    print_tree(root->l_node, indent);
}

/*
* Function: free_tree
* -------------------
*  Frees the allocated memory for the tree (Recursively).
*
*  root: Pointer to the root of the tree
*/
void free_tree(Node* root) {
    if (root == NULL) {
        return; // Base case: nothing to free
    }
    // Recursively free left and right subtrees
    free_tree(root->r_node);
    free_tree(root->l_node);
    // Free the current node
    free(root);
}

/*
* Function: free_heap_nodes
* -------------------------
*  Iterates over the nodes in min-heap object
*  and deletes them from the memory.
*
*  heap: Pointer to the min-heap object.
*
*  returns: Number of freed nodes.
*/
size_t free_heap_nodes(Heap* heap) {
    size_t counter = 0;
    for (size_t i = 0; i < heap->size; i++) {
        Node* node = (Node*) heap->nodes[i];
        free(node);
        counter++;
    }
    return counter;
}

/*
* Function: scale_down
* --------------------
*  Returns an scaled down value.
*
*  value: The value to be scaled down
*  scale: The base value
*
*  returns: Scaled down value.
*/
size_t scale_down(size_t value, size_t scale) {
    size_t result = scale > 1 ? value / scale : value;
    if (result == 0 && scale > 1) result++;
    return result;
}

/*
* Function: write_file_header
* ---------------------------
*  Writes header information to the output file
*
*  output_file: Pointer to the output file
*  frequency_table: Pointer to the frequency table
*
*  returns: If failed (0), on success (1)
*/
int write_file_header(FILE* output_file, size_t* frequency_table) {
    size_t max_value = 0;
    size_t list_size = get_list_size(frequency_table, &max_value);
    if (list_size == 0) {
        fprintf(stderr, "\n[ERROR]: write_file_header() {} -> Frequency table is all zero!\n");
        return 0;
    }
    list_size--; // to avoid using extra byte for the value 256.
    if (fwrite(&list_size, sizeof(unsigned char), 1, output_file) < 1) return 0;
    list_size++;

    HeaderFrequencyTable* header_frequency_table = malloc(list_size * sizeof(HeaderFrequencyTable));
    if (header_frequency_table == NULL) {
        fprintf(stderr, "\n[ERROR]: write_file_header() {} -> Unable to allocate memory for header frequency table!\n");
        return 0;
    }
    size_t header_frequency_table_idx = 0; 

    for (size_t i = 0; i < FREQUENCY_TABLE_SIZE && header_frequency_table_idx < list_size; i++) {
        if (frequency_table[i] > 0) {
            header_frequency_table[header_frequency_table_idx].symbol = (unsigned char) i;
            header_frequency_table[header_frequency_table_idx].frequency = scale_down(frequency_table[i], max_value);
            // header_frequency_table[header_frequency_table_idx].frequency = frequency_table[i] / max_value;
            // if (header_frequency_table[header_frequency_table_idx].frequency == 0) {
            //     header_frequency_table[header_frequency_table_idx].frequency++;
            // }
            header_frequency_table_idx++;
        }
    }
    size_t written_bytes = fwrite(header_frequency_table, sizeof(HeaderFrequencyTable), list_size, output_file);
    if (written_bytes < list_size) {
        fprintf(stderr, "\n[ERROR]: write_file_header() {} -> Unable to write header!\n");
        return 0;
    }
    return 1;
}
/*
* Function: read_file_header
* --------------------------
*  Reads the header information of compressed file.
*
*  input_file: Pointer to the compressed file
*  bit_padding: Pointer to the variable storing remaining bit count
*
*  returns: Frequency table
*/
size_t* read_file_header(FILE* input_file, size_t* list_size, int* bit_padding) {
    size_t* frequency_table = malloc(FREQUENCY_TABLE_SIZE * sizeof(size_t));
    if (frequency_table == NULL) {
        fprintf(stderr, "\n[ERROR]: read_file_header() {} -> Unable to allocate memory for frequency table!\n");
        return NULL;
    }
    // set every value to zero, in order to start counting occurance
    memset(frequency_table, 0, FREQUENCY_TABLE_SIZE * sizeof(size_t)); 

    fseek(input_file, 0, SEEK_SET);
    size_t read_bytes = fread(list_size, sizeof(unsigned char), 1, input_file);
    if (read_bytes < 1) {
        fprintf(stderr, "\n[ERROR]: read_file_header() {} -> File is corrupted!\n");
        return NULL;
    }
    (*list_size)++; // increase list_size by 1, because it was decreased by 1 when it was saved

    unsigned char* read_buffer = malloc(*list_size * sizeof(unsigned char) * 2);
    if (read_buffer == NULL) {
        fprintf(stderr, "\n[ERROR]: read_file_header() {} -> Unable to allocate memory for buffer!\n");
        return NULL;
    }

    size_t header_frequency_table_size = *list_size * 2;
    read_bytes = fread(read_buffer, sizeof(unsigned char), header_frequency_table_size, input_file);
    if (read_bytes < header_frequency_table_size) {
        fprintf(stderr, "\n[ERROR]: read_file_header() {} -> File is corrupted!\n");
        return NULL;
    }

    // Read file's frequency table and import it
    for (size_t i = 0; i < header_frequency_table_size; i += 2) {
        unsigned char symbol = read_buffer[i];
        unsigned char count = read_buffer[i + 1];
        frequency_table[symbol] = count;
    }

    free(read_buffer);

    // Read total encoded bits count at the end of the file
    size_t header_end_pos = ftell(input_file);
    long remainign_bits_count_pos = sizeof(unsigned char);
    fseek(input_file, -1 * remainign_bits_count_pos, SEEK_END);
    if (fread(bit_padding, sizeof(unsigned char), 1, input_file) < 1) {
        fprintf(stderr, "\n[ERROR]: read_file_header() {} -> Unable to read total bit_count from file header!\n");
        return NULL;
    }
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
        // if tree has only one node, instead of zero, write 1 as depth
        code_table[node->symbol].length = depth == 0 ? 1 : depth; 
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
    unsigned char* read_buffer = malloc(READ_BUFFER_SIZE * sizeof(unsigned char));
    if (read_buffer == NULL) {
        fprintf(stderr, "\n[ERROR]: encode() {} -> Unable to allocate memory for buffer!\n");
        return 0;
    }
    size_t bytes_read = 0;
    size_t file_size = get_file_size(input_file);
    size_t processed = 0;
    clock_t start_time = clock();
    fseek(input_file, 0, SEEK_SET);

    while((bytes_read = fread(read_buffer, sizeof(unsigned char), READ_BUFFER_SIZE, input_file)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            unsigned char symbol = read_buffer[i];
            // Encode symbol to huffman bits
            if (code_table[symbol].length > 0) {
                int result = write_bits(bit_writer, code_table[symbol].code, code_table[symbol].length);
                if (result == -1) {
                    free(read_buffer);
                    return 0;
                }
            }
        }
        processed += bytes_read;
        if (processed % (100 * KB) == 0) {
            printf("\rProcessing: %zu/%zu bytes...", processed, file_size);
        }
    }

    // Flush the remaining data in writer to the file
    int result = flush_writer(bit_writer);
    if (result == -1) {
        free(read_buffer);
        return 0;
    }

    clock_t end_time = clock();
    long compressed_file_size = ftell(bit_writer->file);
    int size_diff = file_size - compressed_file_size;
    double compression_rate = (double) abs(size_diff) / file_size * 100;
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("\rFinished processing (%f s): %zu bytes -> %ld bytes (%s%.2f%%)\n", time_spent, file_size, 
           compressed_file_size, size_diff > 0 ? "-" : "+", compression_rate);

    free(read_buffer);
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
*  bit_padding: Number of encoded bits in the last byte of the file
*
*  returns: If failed (0), on success (1)
*/
int decode(FILE *output_file, BitReader *bit_reader, Node* root, int bit_padding) {
    size_t output_buffer_size = READ_BUFFER_SIZE * sizeof(unsigned char);
    unsigned char* output_buffer = malloc(output_buffer_size);
    if (output_buffer == NULL) {
        fprintf(stderr, "\n[ERROR]: encode() {} -> Unable to allocate memory for buffer!\n");
        return 0;
    }
    size_t output_pos = 0;
    size_t file_size = get_file_size(bit_reader->file);
    size_t processed = ftell(bit_reader->file);
    // whole file - header - last byte (bits_padding) = encoded bits
    size_t total_bits = (file_size - processed - 1) * 8 - 8 + bit_padding;
    Node* current = root;
    clock_t start_time = clock();

    while (bit_reader->bits_read <= total_bits) {
        int bit = read_bits(bit_reader);
        if (bit == -1) {
            break;
        }
        current = bit ? current->r_node : current->l_node;
        // Leaf Node:
        if (current->l_node == NULL && current->r_node == NULL) { 
            output_buffer[output_pos++] = current->symbol;
            // Flush output_buffer
            if (output_pos >= output_buffer_size) {
                size_t written_bytes = fwrite(output_buffer, sizeof(unsigned char), output_pos, output_file);
                if (written_bytes < output_pos) {
                    free(output_buffer);
                    return 0;
                }
                output_pos = 0;
            }
            current = root;
        }
        size_t read_bytes = bit_reader->bits_read / 8;
        if (read_bytes % (100 * KB) == 0) {
            printf("\rProcessing: %ld/%ld bytes...", processed + read_bytes, file_size);
        }
    }

    // Flush the remaining data in writer to the file
    if (output_pos > 0) {
        size_t written_bytes = fwrite(output_buffer, sizeof(unsigned char), output_pos, output_file);
        if (written_bytes < output_pos) {
            free(output_buffer);
            return 0;
        }
    }

    clock_t end_time = clock();
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    long compressed_file_size = ftell(bit_reader->file);
    printf("\rFinished Processing (%f s): %zu bytes -> %ld bytes.\n", time_spent, file_size, compressed_file_size);

    free(output_buffer);
    return 1;
}
