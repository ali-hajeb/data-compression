#ifndef MINHEAP_H
#define MINHEAP_H
#include <sys/types.h>

typedef struct node {
    unsigned char symbol;
    size_t frequency;
    struct node* r_node;
    struct node* l_node;
} Node;

typedef struct {
    Node* nodes;
    size_t max_size;
    size_t size;
} Heap;

/*
* Function: get_list_size
* -----------------------
*  Returns the number of non-zero values. Also can obtain the maximum value.
*
*  list: Pointer to the list array.
*  max_value: Pointer to the max_value variable. (Can be NULL).
*
*  returns: Count of non-zero values.
*/
size_t get_list_size(size_t* list, size_t* max_value);

/*
* Function: create_priority_queue
* -------------------------------
*  creates a min-heap structure containing a list of nodes
*
*  list: Pointer to the frequency table
*
*  returns: A pointer to the min-heap structure. If failed, returns NULL
*/
Heap* create_priority_queue(size_t* list); 

/*
* Function build_tree
* -------------------
*  Builds the binary tree of the min-heap
*
*  heap: Pointer to the min-heap
*
*  returns: A pointer to the root of the tree
*/
Node* build_tree(Heap* heap);

/*
* Function: swap
* --------------
*  Swaps the value of two variables.
*
*  p1: Pointer to the first value
*  p2: Pointer to the second value
*/
void swap(Node* p1, Node* p2);

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
ssize_t heap_insert(Heap* heap, Node* node);

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
*  node: The node to be indexed
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
Node* heap_extract(Heap* heap);

/*
* Function combine_nodes
* ----------------------
*  Combines the nodes and returns a new node
*
*  n1: First node
*  n2: Second node
*
*  returns: New node with 2 childeren
*/
Node* combine_nodes(Node* n1, Node* n2);

/*
* Function: print_tree
* --------------------
*  Prints the tree (Recursively) .
*
*  root: Pointer to the root of the tree
*  indent: Number of space indentation after each branch
*/
void print_tree(Node* root, int indent); 

/*
* Function: free_tree
* -------------------
*  Frees the allocated memory for the tree (Recursively).
*
*  root: Pointer to the root of the tree
*/
void free_tree(Node* root);
void print_heap(Heap* heap, const char* title);
#endif
