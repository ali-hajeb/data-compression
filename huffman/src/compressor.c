#include "../include/constants.h"
#include "../include/minheap.h"
#include "../include/compressor.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

int fill_minheap(size_t* frequency_table, Heap* priority_queue) {
    size_t max_count = 0;
    size_t heap_size = get_list_size(frequency_table, &max_count);
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
    return heap_size;
}

void heap_sorter(Heap* heap, size_t* parent_index, size_t* index) {
    Node selected_node = ((Node*) heap->nodes)[*index];
    Node parent_node = ((Node*) heap->nodes)[*parent_index];

    if ((selected_node.frequency < parent_node.frequency) 
        || (selected_node.frequency == parent_node.frequency 
            && selected_node.symbol < parent_node.symbol)) {
        swap(&selected_node, &parent_node, sizeof(Node));
        *index = sort_heap_node(heap, *index);
    }
}
