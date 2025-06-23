#include "../include/minheap.h"
#include "../include/constants.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
size_t get_list_size(size_t* list, size_t* max_value) {
    size_t list_size = 0;
    for (size_t i = 0; i < FREQUENCY_TABLE_SIZE; i++) {
        if (list[i] != 0) {
            list_size++;
            if (list[i] > *max_value) {
                *max_value = list[i];
            }
        }
    }
    *max_value /= 255;
    *max_value += 1;
    return list_size;
}

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
                            void (*sorter)(struct heap* heap, size_t* parent_index, size_t* index)) {

    // Initialize the min-heap
    Heap* priority_queue = malloc(sizeof(Heap));
    if (priority_queue == NULL) {
        fprintf(stderr, "\n[ERROR]: create_priority_queue() {} -> Unable to allocate memory for priority_queue heap!\n");
        return NULL;
    }

    priority_queue->nodes = malloc(initial_capacity * node_size);
    if (priority_queue->nodes == NULL) {
        fprintf(stderr, "\n[ERROR]: create_priority_queue() {} -> Unable to allocate memory for priority_queue nodes!\n");
        free(priority_queue);
        return NULL;
    }

    priority_queue->sorter = sorter;
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

    if(memcpy(&heap->nodes[(heap->size)++], node, heap->node_size) == NULL) {
        return -1;
    }
    size_t node_index = sort_heap_node(heap, heap->size - 1);
    return node_index;
}

/*
* Function: sort_heap_node
* ------------------------
*  Finds the appropriate index for the node in the min-heap
*
*  heap: Pointer to the min-heap
*  index: index of the node to be sorted
*  sorter: Pointer to the function that does the sorting
*
*  returns: The index of the node.
*/
ssize_t sort_heap_node(Heap* heap, size_t index) {
    if (index == 0) {
        return 0;
    }

    size_t parent_index = (index - 1) / 2;
    size_t new_index = index;

    heap->sorter(heap, &parent_index, &index);
    return new_index;
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
    if (memcpy(node, &heap->nodes[0], heap->node_size) 
        || memcpy(&heap->nodes[0], &heap->nodes[heap->size], heap->node_size)) {
        fprintf(stderr, "\n[ERROR]: heap_extract() {} -> Unable to extract the node from the heap!\n");
        return NULL;
    }

    // Reduce heap size
    heap->size--;

    // Re-sort heap
    sort_heap(heap, 0);

    return node;
}

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
size_t sort_heap(Heap* heap, size_t index) {
    size_t left_child = index * 2 + 1;
    size_t right_child = index * 2 + 2;
    size_t min_index = index;

    if (left_child < heap->size && 
        (heap->nodes[left_child].frequency < heap->nodes[min_index].frequency 
        || (heap->nodes[left_child].frequency == heap->nodes[min_index].frequency 
            && heap->nodes[left_child].symbol < heap->nodes[min_index].symbol))) {
        min_index = left_child;
    }
    if (right_child < heap->size &&
        (heap->nodes[right_child].frequency < heap->nodes[min_index].frequency 
        || (heap->nodes[right_child].frequency == heap->nodes[min_index].frequency 
            && heap->nodes[right_child].symbol < heap->nodes[min_index].symbol))) {
        min_index = right_child;
    }

    if (min_index != index) {
        swap(&heap->nodes[index], &heap->nodes[min_index]);
        min_index = sort_heap(heap, min_index);
    }
    return min_index;
}

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
Node* combine_nodes(Node* n1, Node* n2) {
    Node* new_node = malloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "\n[ERROR]: combine_nodes() {} -> Unable to combine two nodes!\n");
        return NULL;
    }

    new_node->symbol = 0xFF;
    new_node->frequency = n1->frequency + n2->frequency;
    new_node->l_node = n1;
    new_node->r_node = n2;

    return new_node;
}

/*
* Function build_tree
* -------------------
*  Builds the binary tree of the min-heap
*
*  heap: Pointer to the min-heap
*
*  returns: A pointer to the root of the tree
*/
Node* build_tree(Heap* heap) {
    while (heap->size >= 2) {
        Node* n1 = heap_extract(heap);
        Node* n2 = heap_extract(heap);
        if (n1 == NULL || n2 == NULL) {
            return NULL;
        }

        Node* new_node = combine_nodes(n1, n2);
        if (new_node == NULL) {
            free(n1);
            free(n2);
            return NULL;
        }

        ssize_t index = heap_insert(heap, new_node);
        if (index == -1) {
            free(n1);
            free(n2);
            free(new_node);
            return NULL;
        }
        free(new_node);
    }

    return heap_extract(heap);
}

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
void* swap(void* p1, void* p2, size_t value_size) {
    void* temp = malloc(value_size);
    if (temp == NULL) {
        return NULL;
    }
    if (memcpy(temp, p1, value_size) == NULL
        || memcpy(p1, p2, value_size) == NULL
        || memcpy(p2, temp, value_size) == NULL) {
        free(temp);
        return NULL;
    }

    free(temp);
    return p2;
}
/*
* Function: print_tree
* --------------------
*  Prints the tree (Recursively) .
*
*  root: Pointer to the root of the tree
*  indent: Number of space indentation after each branch
*/
void print_tree(Node* root, int indent) {
    printf("%*s[%p (%c): (%zu)] ->\n", indent, " ", root, root->symbol, root->frequency);

    if (root->r_node == NULL && root->l_node == NULL) {
        return;
    } 

    indent += 5;
    print_tree(root->r_node, indent);
    print_tree(root->l_node, indent);
}

void print_heap(Heap* heap, const char* title) {
    printf("\n===========| %s |===========\n", title);
    for (size_t i = 0; i < heap->size; i++) {
        printf("[%2X]: %c (%zu)\n", heap->nodes[i].symbol, heap->nodes[i].symbol, heap->nodes[i].frequency);
    }
    printf("============================\n");
}
/*
* Function: free_tree
* -------------------
*  Frees the allocated memory for the tree (Recursively).
*
*  root: Pointer to the root of the tree
*/
void free_tree(Node* root) {
    if (root == NULL) {
        return; // Base case: nothing to free
    }
    // Recursively free left and right subtrees
    free_tree(root->r_node);
    free_tree(root->l_node);
    // Free the current node
    free(root);
}

