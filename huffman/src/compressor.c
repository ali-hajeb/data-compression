#include "../include/constants.h"
#include "../include/minheap.h"
#include "../include/huffman.h"
#include "../include/compressor.h"
#include "../include/resources.h"

#include <stdio.h>
#include <stdlib.h>

/*
* Function: fill_minheap
* ----------------------
* Creates a new node for every non-zero value in frequency table
* and inserts it to the min-heap
*
* frequency_table: Pointer to the frequency table
* priority_queue: Pointer to the min-heap object
*
* returns: Count of inserted nodes.
*          If failed, returns (-1).
*/
int fill_minheap(size_t* frequency_table, Heap* priority_queue, size_t max_count) {
    for (size_t i = 0; i < FREQUENCY_TABLE_SIZE; i++) {
        if (frequency_table[i] != 0) {
            // Scale down the frequency table
            size_t frequency = frequency_table[i] / max_count;
            if (frequency == 0) frequency = 1;

            // Create a new node
            Node node = { .symbol = (unsigned char) i, .frequency = frequency, .l_node = NULL, .r_node = NULL };

            // Insert the new node to heap node list
            ssize_t node_index = heap_insert(priority_queue, &node);
            if (node_index == -1) {
                fprintf(stderr, "\n[ERROR]: create_priority_queue() {} -> Heap insert failed!\n");
                return -1;
            }
        }
    }
    return priority_queue->size;
}

/*
* Function: compress
* ------------------
* Compresses the input file using huffman coding
*
* input_file: Pointer to the input_file
* output_file: Pointer to the output_file
*
* returns: If failed (0), On success (1)
*/
int compress(FILE* input_file, FILE* output_file) {
    // Initialize resource management system
    Resources resource = resources_init(5);
    // Generate frequency table
    size_t* frequency_table = count_run(input_file);
    if (frequency_table == NULL || resources_add(&resource, frequency_table) == 0) {
        resources_cleanup(&resource);
        return 0;
    }

    printf("here1\n");

    size_t max_count = 0;
    size_t heap_capacity = get_list_size(frequency_table, &max_count);

    // Create a min-heap structure for nodes
    Heap* priority_queue = create_priority_queue(heap_capacity, sizeof(Node), &compare_nodes);
    if (priority_queue == NULL) {
        resources_cleanup(&resource);
        return 0;
    }

    printf("here2\n");
    int heap_size = fill_minheap(frequency_table, priority_queue, max_count);
    if (heap_size <= 0) {
        free_heap(priority_queue);
        resources_cleanup(&resource);
        return 0;
    }
    
    // Create a binary huffman tree
    Node* root = build_tree(priority_queue);
    if (root == NULL) {
        free_heap(priority_queue);
        resources_cleanup(&resource);
        return 0;
    }

    print_tree(root, 0);

    printf("here3\n");
    // Create a table for the huffman encoded symbols
    Code* code_table = malloc(FREQUENCY_TABLE_SIZE * sizeof(Code));
    if (code_table == NULL || resources_add(&resource, code_table) == 0) {
        free_tree(root);
        free_heap(priority_queue);
        resources_cleanup(&resource);
        return 0;
    }
    
    printf("here4\n");
    generate_huffman_code(code_table, 0, 0, root);

    printf("here5\n");
    BitWriter* bit_writer = init_writer(output_file);
    if (bit_writer == NULL || resources_add(&resource, bit_writer) == 0) {
        free_tree(root);
        free_heap(priority_queue);
        resources_cleanup(&resource);
        return 0;
    }

    printf("here6\n");
    // Write file header (Read the readme file for more information about the compressed file structure)
    int header_res = write_file_header(output_file, frequency_table);
    if (header_res == 0) {
        free_tree(root);
        free_heap(priority_queue);
        resources_cleanup(&resource);
        return 0;
    }

    printf("here7\n");
    // Encode and compress file
    int result = encode(input_file, bit_writer, code_table);
    if (result == 0) {
        free_tree(root);
        free_heap(priority_queue);
        resources_cleanup(&resource);
        return 0;
    }
    printf("here8\n");

    // Write the total bits
    size_t total_bits = bit_writer->total_bits;
    size_t res = fwrite(&total_bits, sizeof(size_t), 1, output_file);

    free_tree(root);
    free_heap(priority_queue);
    resources_cleanup(&resource);
    return res;
}

/*
* Function: decompress
* ------------------
* Decompresses the input file using huffman coding
*
* input_file: Pointer to the input_file
* output_file: Pointer to the output_file
*
* returns: If failed (0), On success (1)
*/
int decompress(FILE* input_file, FILE* output_file) {
    // Initialize resource management system
    Resources resource = resources_init(4);
    if (resource.pointers == NULL) {
        return 0;
    }

    BitReader* bit_reader = init_reader(input_file);
    if (bit_reader == NULL || resources_add(&resource, bit_reader) == 0) {
        resources_cleanup(&resource);
        return 0;
    }

    size_t total_bits = 0;
    size_t list_size = 0;
    size_t* frequencty_table = read_file_header(input_file, &list_size,&total_bits);
    if (frequencty_table == NULL || resources_add(&resource, frequencty_table) == 0) {
        resources_cleanup(&resource);
        return 0;
    }

    Heap* priority_queue = create_priority_queue(list_size, sizeof(Node), &compare_nodes);
    if (priority_queue == NULL) {
        resources_cleanup(&resource);
        return 0;
    }
    
    Node* root = build_tree(priority_queue);
    if (root == NULL) {
        free_heap(priority_queue);
        resources_cleanup(&resource);
        return 0;
    }

    int result = decode(output_file, bit_reader, root, total_bits);

    free_tree(root);
    free_heap(priority_queue);
    resources_cleanup(&resource);
    return result;
}

void print_heap(Heap* heap, const char* title) {
    Node* nodes = (Node*) heap->nodes;
    printf("\n===========| %s |===========\n", title);
    for (size_t i = 0; i < heap->size; i++) {
        printf("[%2X]: %c (%zu)\n", nodes[i].symbol, nodes[i].symbol, nodes[i].frequency);
    }
    printf("============================\n");
}
