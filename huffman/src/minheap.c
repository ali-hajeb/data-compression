#include "../include/constants.h"
#include "../include/minheap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
* Function: create_priority_queue
* -------------------------------
*  creates a min-heap structure containing a list of nodes
*
*  initial_capacity: Initial number of nodes
*  node_size: Size of node structure
*  sorter: Pointer to the function that does the sorting
*
*  returns: A pointer to the min-heap structure. If failed, returns NULL
*/
Heap* create_priority_queue(size_t initial_capacity, size_t node_size,
                            int (*compare)(const void* a, const void* b)) {

    // Initialize the min-heap
    Heap* priority_queue = malloc(sizeof(Heap));
    if (priority_queue == NULL) {
        fprintf(stderr, "\n[ERROR]: create_priority_queue() {} -> Unable to allocate memory for priority_queue heap!\n");
        return NULL;
    }

    priority_queue->nodes = malloc(initial_capacity);
    if (priority_queue->nodes == NULL) {
        fprintf(stderr, "\n[ERROR]: create_priority_queue() {} -> Unable to allocate memory for priority_queue nodes!\n");
        free(priority_queue);
        return NULL;
    }

    priority_queue->compare = compare;
    priority_queue->node_size = node_size;
    priority_queue->size = 0;
    priority_queue->max_size = initial_capacity;

    return priority_queue;
}

/*
* Function: heap_insert
* ---------------------
*  Inserts the node in the min-heap
*
*  heap: Pointer to the min-heap
*  node: The node to be inserted
*
*  returns: The index of the node in the heap. If failed, returns -1.
*/
ssize_t heap_insert(Heap* heap, void* node) {
    if (heap->size >= heap->max_size) {
        return -1;
    }

    if(memcpy(heap->nodes[(heap->size)++], node, heap->node_size) == NULL) {
        return -1;
    }
    size_t node_index = heapify_up(heap, heap->size - 1);
    return node_index;
}

/*
* Function: heapify_up
* --------------------
*  Puts the selected node at the appropiate index
*
*  heap: Pointer to the min-heap object
*  index: Index of the selected node
*
*  returns: New index of the selected node.
*           If failed, returns (-1)
*/
ssize_t heapify_up(Heap* heap, size_t index) {
    while (index > 0) {
        size_t parent_idx = (index - 1) / 2;
        void* index_node = heap->nodes[index];
        void* parent_node = heap->nodes[parent_idx];

        if (heap->compare(index_node, parent_node) < 0) {
            void* result = swap(index_node, parent_node);
            if (result == NULL) {
                return -1;
            }
            index = parent_idx;
        }
        else {
            break;
        }
    }
    return index;
}

/*
* Function: heapify_down
* ----------------------
*  Sorts the min-heap from top to down
*
*  heap: Pointer to the min-heap object
*  index: Index of the starting node
*
*  returns: New index of the start node.
*           If failed, returns (-1)
*/
ssize_t heapify_down(Heap* heap, size_t index) {
    size_t left_child = index * 2 + 1;
    size_t right_child = index * 2 + 2;
    size_t min_index = index;

    void* left_child_node = heap->nodes[left_child];
    void* right_child_node = heap->nodes[right_child];
    void* index_node = heap->nodes[min_index];
    void* min_index_node = index_node;

    if (left_child < heap->size && heap->compare(left_child_node, min_index_node)) {
        min_index = left_child;
        min_index_node = left_child_node;
    }
    if (right_child < heap->size && heap->compare(right_child_node, min_index_node)) {
        min_index = right_child;
        min_index_node = right_child_node;
    }

    if (min_index != index) {
        void* result = swap(index_node, min_index_node);
        if (result == NULL) {
            return -1;
        }
        min_index = heapify_down(heap, min_index);
    }
    return min_index;
}

/*
* Function heap_extract
* ---------------------
*  Extracts the top node (minimum value)
*
*  heap: Pointer to the min-heap
*
*  returns: Pointer to the top node
*/
void* heap_extract(Heap* heap) {
    if (heap->size == 0) {
        fprintf(stderr, "\n[ERROR]: heap_extract() {} -> Heap is empty!\n");
        return NULL;
    }
    
    void* node = malloc(heap->node_size);
    if (node == NULL) {
        fprintf(stderr, "\n[ERROR]: heap_extract() {} -> Unable to allocate memory for the node!\n");
        return NULL;
    }

    // The last node will be the first after extraction
    if (memcpy(node, heap->nodes[0], heap->node_size) == NULL
        || memcpy(heap->nodes[0], heap->nodes[heap->size], heap->node_size) == NULL) {
        fprintf(stderr, "\n[ERROR]: heap_extract() {} -> Unable to extract the node from the heap!\n");
        return NULL;
    }

    // Reduce heap size
    heap->size--;

    // Re-sort heap
    heapify_down(heap, 0);

    return node;
}

/*
* Function: swap
* --------------
*  Swaps the value of two variables.
*
*  p1: Pointer to the first value
*  p2: Pointer to the second value
*
*  returns: Void pointer to the second value.
*           If fails, returns NULL
*/
void* swap(void** p1, void** p2) {
    void* temp = p1;
    *p1 = *p2;
    *p2 = temp;
    return p2;
}

/*
* Function: free_heap
* -------------------
*  Frees the min-heap from the memory
*
*  heap: Pointer to the min-heap object
*/
void free_heap(Heap* heap) {
    if (heap == NULL) {
        return;
    }

    free(heap->nodes);
    free(heap);
}
