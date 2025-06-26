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
*  compare: Pointer to the function that compares nodes priority
*
*  returns: A pointer to the min-heap structure. If failed, returns NULL
*/
Heap* create_priority_queue(size_t initial_capacity, int (*compare)(const void* a, const void* b)) {

    // Initialize the min-heap
    Heap* priority_queue = malloc(sizeof(Heap));
    if (priority_queue == NULL) {
        fprintf(stderr, "\n[ERROR]: create_priority_queue() {} -> Unable to allocate memory for priority_queue heap!\n");
        return NULL;
    }

    priority_queue->nodes = malloc(initial_capacity * sizeof(void*));
    if (priority_queue->nodes == NULL) {
        fprintf(stderr, "\n[ERROR]: create_priority_queue() {} -> Unable to allocate memory for priority_queue nodes!\n");
        free(priority_queue);
        return NULL;
    }

    priority_queue->compare = compare;
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
    if (heap == NULL || node == NULL || heap->size >= heap->max_size) {
        return -1;
    }

    // printf("****\n");
    heap->nodes[heap->size++] = node;
    // for (size_t i = 0; i < heap->size; i++) {
    //     printf("\t[%zu] %2X: %zu\n", i, ((Node*) heap->nodes[i])->symbol, ((Node*) heap->nodes[i])->frequency);
    // }
    ssize_t node_index = heapify_up(heap, heap->size - 1);
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
    if (heap == NULL || index >= heap->size) {
        return -1;
    }
    while (index > 0) {
        size_t parent_idx = (index - 1) / 2;

        if (heap->compare(heap->nodes[index], heap->nodes[parent_idx]) < 0) {
            swap(&heap->nodes[index], &heap->nodes[parent_idx]);
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
    size_t left_child_idx = index * 2 + 1;
    size_t right_child_idx = index * 2 + 2;
    size_t min_index = index;

    if (left_child_idx < heap->size && heap->compare(heap->nodes[left_child_idx], heap->nodes[index]) < 0) {
        min_index = left_child_idx;
    }
    if (right_child_idx < heap->size && heap->compare(heap->nodes[right_child_idx], heap->nodes[min_index]) < 0) {
        min_index = right_child_idx;
    }

    if (min_index != index) {
        swap(&heap->nodes[index], &heap->nodes[min_index]);
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
    if (heap == NULL || heap->size == 0) {
        fprintf(stderr, "\n[ERROR]: heap_extract() {} -> Heap is empty!\n");
        return NULL;
    }

    // void* node = malloc(sizeof(void*));
    // if (node == NULL) {
    //     fprintf(stderr, "\n[ERROR]: heap_extract() {} -> Unable to allocate memory for the node!\n");
    //     return NULL;
    // }
    //
    // node = &heap->nodes[0];

    void* node = heap->nodes[0];
    // Reduce heap size
    heap->size--;
    if (heap->size > 0) {
        heap->nodes[0] = heap->nodes[heap->size];
 
        // Clear last pointer
        heap->nodes[heap->size] = NULL; 

        // Re-sort the min-heap
        heapify_down(heap, 0);
    } else {
        // Clear root if heap is empty
        heap->nodes[0] = NULL; 
    }
    return node;
}

/*
* Function: swap
* --------------
*  Swaps the value of two variables.
*
*  p1: Pointer to the first value
*  p2: Pointer to the second value
*/
void swap(void** p1, void** p2) {
    void* temp = *p1;
    *p1 = *p2;
    *p2 = temp;
}

/*
* Function: free_heap
* -------------------
*  Frees the min-heap from the memory.
*  Note: This function does not free the nodes!
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
