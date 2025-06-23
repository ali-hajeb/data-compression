#ifndef MINHEAP_H
#define MINHEAP_H
#include <sys/types.h>


typedef struct heap {
    void* nodes;
    size_t node_size;
    void (*sorter)(struct heap* heap, size_t* parent_index, size_t* index); 
    size_t max_size;
    size_t size;
} Heap;


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
                            void (*sorter)(struct heap* heap, size_t* parent_index, size_t* index));

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
* Function: sort_heap
* -------------------
*  Sorts the min-heap based on the node values
*
*  heap: Pointer to the min-heap
*  index: Index of the node to start the sort
*
*  returns: Index of the node
*/
size_t sort_heap(Heap* heap, size_t index); 

/*
* Function: sort_heap_node
* ------------------------
*  Finds the appropriate index for the node in the min-heap
*
*  heap: Pointer to the min-heap
*  index: index of the node to be sorted
*
*  returns: The index of the node.
*/
ssize_t sort_heap_node(Heap* heap, size_t index);

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
*  value_size: Size of the value
*
*  returns: Void pointer to the second value.
*           If fails, returns NULL
*/
void* swap(void* p1, void* p2, size_t value_size);

void print_heap(Heap* heap, const char* title);
#endif
