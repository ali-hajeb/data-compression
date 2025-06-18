#include "../include/minheap.h"
#include "../include/constants.h"

#include <stdio.h>
#include <stdlib.h>

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
*  list: Pointer to the frequency table
*
*  returns: A pointer to the min-heap structure. If failed, returns NULL
*/
Heap* create_priority_queue(size_t* list) {
    size_t max_count = 0;
    size_t heap_size = get_list_size(list, &max_count);

    // Initialize the min-heap
    Heap* priority_queue = malloc(sizeof(Heap));
    if (priority_queue == NULL) {
        fprintf(stderr, "\n[ERROR]: create_priority_queue() {} -> Unable to allocate memory for priority_queue heap!\n");
        return NULL;
    }

    priority_queue->nodes = malloc(heap_size * sizeof(Node));
    if (priority_queue->nodes == NULL) {
        fprintf(stderr, "\n[ERROR]: create_priority_queue() {} -> Unable to allocate memory for priority_queue nodes!\n");
        free(priority_queue);
        return NULL;
    }

    priority_queue->size = 0;
    priority_queue->max_size = heap_size;

    for (size_t i = 0; i < FREQUENCY_TABLE_SIZE; i++) {
        if (list[i] != 0) {
            // Scale down the frequency table
            size_t frequency = list[i] / max_count;
            if (frequency == 0) frequency = 1;

            // Create a new node
            Node node = { .symbol = (unsigned char) i, .frequency = frequency, .l_node = NULL, .r_node = NULL };

            // Insert the new node to heap node list
            ssize_t node_index = heap_insert(priority_queue, &node);
            if (node_index == -1) {
                fprintf(stderr, "\n[ERROR]: create_priority_queue() {} -> Heap insert failed!\n");
                free(priority_queue->nodes);
                free(priority_queue);
                return NULL;
            }
        }
    }
    return priority_queue;
}

/*
* Function: swap
* --------------
*  Swaps the value of two variables.
*
*  p1: Pointer to the first value
*  p2: Pointer to the second value
*/
void swap(Node* p1, Node* p2) {
    Node temp = *p1;
    *p1 = *p2;
    *p2 = temp;
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
ssize_t heap_insert(Heap* heap, Node* node) {
    if (heap->size >= heap->max_size) {
        return -1;
    }

    heap->nodes[heap->size++] = *node;
    size_t node_index = sort_heap_node(heap, heap->size - 1);
    return node_index;
}

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
ssize_t sort_heap_node(Heap* heap, size_t index) {
    if (index == 0) {
        return 0;
    }

    size_t parent_index = (index - 1) / 2;
    size_t new_index = index;

    if ((heap->nodes[index].frequency < heap->nodes[parent_index].frequency) 
        || (heap->nodes[index].frequency == heap->nodes[parent_index].frequency 
            && heap->nodes[index].symbol < heap->nodes[parent_index].symbol)) {
        swap(&heap->nodes[index], &heap->nodes[parent_index]);
        new_index = sort_heap_node(heap, new_index);
    }
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
Node* heap_extract(Heap* heap) {
    if (heap->size == 0) {
        fprintf(stderr, "\n[ERROR]: heap_extract() {} -> Heap is empty!\n");
        return NULL;
    }
    
    Node* node = malloc(sizeof(Node));
    if (node == NULL) {
        fprintf(stderr, "\n[ERROR]: heap_extract() {} -> Unable to allocate memory for the node!\n");
        return NULL;
    }

    *node = heap->nodes[0];
    // The last node will be the first after extraction
    heap->nodes[0] = heap->nodes[--heap->size];

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

