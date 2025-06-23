#ifndef COMPRESSOR_H
#define COMPRESSOR_H
#include "minheap.h"

#include <stddef.h>

typedef struct node {
    unsigned char symbol;
    size_t frequency;
    struct node* r_node;
    struct node* l_node;
} Node;

void heap_sorter(Heap* heap, size_t* parent_index, size_t* index) {

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
#endif
