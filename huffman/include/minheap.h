#ifndef MINHEAP_H
#define MINHEAP_H
#include <sys/types.h>


typedef struct heap {
    int (*compare)(const void* a, const void* b); 
    void** nodes;
    size_t node_size;
    size_t max_size;
    size_t size;
} Heap;


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
Heap* create_priority_queue(size_t initial_capacity, int (*compare)(const void* a, const void* b));

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
ssize_t heap_insert(Heap* heap, void* node);

/*
* Function: heapify_up
* --------------------
*  Puts the selected node at the appropiate  index
*
*  heap: Pointer to the min-heap object
*  index: Index of the selected node
*
*  returns: New index of the selected node.
*           if failed, returns (-1)
*/
ssize_t heapify_up(Heap* heap, size_t index);

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
ssize_t heapify_down(Heap* heap, size_t index);

/*
* Function heap_extract
* ---------------------
*  Extracts the top node (minimum value)
*
*  heap: Pointer to the min-heap
*
*  returns: Pointer to the top node
*/
void* heap_extract(Heap* heap);

/*
* Function: swap
* --------------
*  Swaps the value of two variables.
*
*  p1: Pointer to the first value
*  p2: Pointer to the second value
*/
void swap(void** p1, void** p2);

/*
* Function: free_heap
* -------------------
*  Frees the min-heap from the memory
*
*  heap: Pointer to the min-heap object
*/
void free_heap(Heap* heap);
#endif
