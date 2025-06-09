#ifndef MINHEAP_H
#define MINHEAP_H
#include <corecrt.h>

typedef struct {
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

Heap* create_priority_queue(size_t* list); 
Node* build_tree(Heap* heap);

// MIN_HEAP helpers
void swap(Node* p1, Node* p2);
ssize_t heap_insert(Heap* heap, Node* node);
size_t sort_heap(Heap* heap, size_t index); 
ssize_t sort_heap_node(Heap* heap, size_t index);
Node* heap_extract(Heap* heap);
Node* combine_nodes(Node* n1, Node* n2);
void print_heap(Heap* heap, const char* title);

// TREE helpers
void print_tree(Node* root, int indent); 
void free_tree(Node* root);
#endif
