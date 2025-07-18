#include "../include/constants.h"
#include "../include/minheap.h"
#include "../include/huffman.h"
#include "../include/compressor.h"
#include "../include/resources.h"
#include "../include/utils.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
* Function: fill_minheap
* ----------------------
* Creates a new node for every non-zero value in frequency table
* and inserts it to the min-heap.
*
* frequency_table: Pointer to the frequency table.
* priority_queue: Pointer to the min-heap object.
* max_count: If > 0, scales down the frequency
*
* returns: Count of inserted nodes.
*/
ssize_t fill_minheap(size_t* frequency_table, Heap* priority_queue, size_t max_count) {
    if (frequency_table == NULL || priority_queue == NULL) {
        err("fill_minheap", "Frequency table and/or priority queue is NULL!");
        return -1;
    }

    for (size_t i = 0; i < FREQUENCY_TABLE_SIZE; i++) {
        if (frequency_table[i] != 0) {
            // Create a new node
            Node* node = malloc(sizeof(Node));
            if (node == NULL) {
                err("fill_minheap", "Unable to allocate memory for the node!");
                return priority_queue->size;
            }
            node->symbol = (unsigned char) i;
            node->frequency = scale_down(frequency_table[i], max_count);
            if (node->frequency == 0) {
                node->frequency++;
            }
            node->l_node = node->r_node = NULL;

            // Insert the new node to heap node list
            ssize_t node_index = heap_insert(priority_queue, node);
            if (node_index == -1) {
                err("fill_minheap", "Heap insert failed!");
                return priority_queue->size;
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
    if (input_file == NULL || output_file == NULL) {
        err("compress", "Input/output file is NULL!\n");
        return 0;
    }
    // Initialize resource management system
    Resources resource = resources_init(4);
    // Generate frequency table
    size_t* frequency_table = count_run(input_file);
    if (frequency_table == NULL || resources_add(&resource, frequency_table) == 0) {
        resources_cleanup(&resource);
        return 0;
    }

    size_t max_count = 0;
    size_t heap_capacity = get_list_size(frequency_table, &max_count);

    // Create a min-heap structure for nodes
    Heap* priority_queue = create_priority_queue(heap_capacity, &compare_nodes);
    if (priority_queue == NULL) {
        resources_cleanup(&resource);
        return 0;
    }

    size_t heap_size = fill_minheap(frequency_table, priority_queue, max_count);
    if (heap_size < heap_capacity) {
        free_heap_nodes(priority_queue);
        free_heap(priority_queue);
        resources_cleanup(&resource);
        return 0;
    }
    
    // Create a binary huffman tree
    Node* root = build_tree(priority_queue);
    if (root == NULL) {
        free_heap_nodes(priority_queue);
        free_heap(priority_queue);
        resources_cleanup(&resource);
        return 0;
    }

    // Create a table for the huffman encoded symbols
    Code* code_table = malloc(FREQUENCY_TABLE_SIZE * sizeof(Code));
    if (code_table == NULL || resources_add(&resource, code_table) == 0) {
        free_tree(root);
        free_heap(priority_queue);
        resources_cleanup(&resource);
        return 0;
    }
    
    generate_huffman_code(code_table, 0, 0, root);

    BitWriter* bit_writer = init_writer(output_file);
    if (bit_writer == NULL || resources_add(&resource, bit_writer->buffer) == 0 
        || resources_add(&resource, bit_writer) == 0) {
        free_tree(root);
        free_heap(priority_queue);
        resources_cleanup(&resource);
        return 0;
    }

    // Write file header (Read the readme file for more information about the compressed file structure)
    int header_res = write_file_header(output_file, frequency_table);
    if (header_res == 0) {
        free_tree(root);
        free_heap(priority_queue);
        resources_cleanup(&resource);
        return 0;
    }

    // Encode and compress file
    int result = encode(input_file, bit_writer, code_table);
    if (result == 0) {
        free_tree(root);
        free_heap(priority_queue);
        resources_cleanup(&resource);
        return 0;
    }

    // Write the total bits
    // size_t total_bits = bit_writer->total_bits;
    // size_t res = fwrite(&total_bits, sizeof(size_t), 1, output_file);
    size_t remaining_bits = bit_writer->total_bits % 8;
    size_t res = fwrite(&remaining_bits, sizeof(unsigned char), 1, output_file);

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
    Resources resource = resources_init(3);
    if (resource.pointers == NULL) {
        return 0;
    }

    BitReader* bit_reader = init_reader(input_file);
    if (bit_reader == NULL || resources_add(&resource, bit_reader->buffer) == 0
        || resources_add(&resource, bit_reader) == 0) {
        resources_cleanup(&resource);
        return 0;
    }

    // size_t total_bits = 0;
    int bit_padding = 0;
    size_t list_size = 0;
    size_t* frequency_table = read_file_header(input_file, &list_size, &bit_padding);
    if (frequency_table == NULL || resources_add(&resource, frequency_table) == 0) {
        resources_cleanup(&resource);
        return 0;
    }

    Heap* priority_queue = create_priority_queue(list_size, &compare_nodes);
    if (priority_queue == NULL) {
        resources_cleanup(&resource);
        return 0;
    }

    size_t heap_size = fill_minheap(frequency_table, priority_queue, 0);
    if (heap_size < list_size) {
        free_heap_nodes(priority_queue);
        free_heap(priority_queue);
        resources_cleanup(&resource);
        return 0;
    }

    Node* root = build_tree(priority_queue);
    if (root == NULL) {
        free_heap_nodes(priority_queue);
        free_heap(priority_queue);
        resources_cleanup(&resource);
        return 0;
    }

    int result = decode(output_file, bit_reader, root, bit_padding);

    free_tree(root);
    free_heap(priority_queue);
    resources_cleanup(&resource);
    return result;
}

void print_heap(Heap* heap, const char* title) {
    Node** nodes = (Node**) heap->nodes;
    printf("\n===========| %s |===========\n", title);
    for (size_t i = 0; i < heap->size; i++) {
        printf("[%02X]: %c (%zu)\n", nodes[i]->symbol, nodes[i]->symbol, nodes[i]->frequency);
    }
    printf("============================\n");
}
