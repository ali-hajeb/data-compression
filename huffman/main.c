#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 0
#define READ_BUFFER_SIZE 256
#define FREQUENCY_TABLE_SIZE 256

size_t FREQUENCY_TABLE[FREQUENCY_TABLE_SIZE];

typedef struct node {
    unsigned char symbol;
    size_t frequency;
    struct node* r_node;
    struct node* l_node;
} Node;

typedef struct heap {
    Node* nodes;
    size_t max_size;
    size_t size;
} Heap;

void print_heap(Heap* heap, const char* title) {
    printf("============| %s |=================\n", title);
    for (size_t i = 0; i < heap->size; i++) {
        printf("[%2X]: %c (%zu)\n", heap->nodes[i].symbol, heap->nodes[i].symbol, heap->nodes[i].frequency);
    }
    printf("=============================\n");
}

FILE* open_file(const char* path, const char* mode);
size_t* count_run(FILE* file);
ssize_t sort(Node* list, size_t size);
void swap(Node* p1, Node* p2);

ssize_t heap_insert(Heap* heap, Node* node);
size_t sort_heap(Heap* heap, size_t index); 
ssize_t sort_heap_node(Heap* heap, size_t index);
Heap* create_priority_queue(size_t* list); 
Node* heap_extract(Heap* heap);

Node* combine_nodes(Node* n1, Node* n2);
Node* build_tree(Node* list, size_t last_index);
Node* build_heap_tree(Heap* heap);
void print_tree(Node* root, int indent); 
void free_tree(Node* root);
void output_file();

void encode();
void decode();

int main(void) {
    // FILE* file = open_file("./pic.bmp", "rb");
    FILE* file = open_file("./test", "rb");
    if (file == NULL) return EXIT_FAILURE;
    
    size_t* freq_table = count_run(file);
    if (freq_table == NULL) {
        fclose(file);
        return EXIT_FAILURE;
    }

    Heap* pq = create_priority_queue(freq_table);
    if (pq == NULL) {
        free(freq_table);
        fclose(file);
        return EXIT_FAILURE;
    }

    if (DEBUG) print_heap(pq, "priority_queue");

    Node* root = build_heap_tree(pq);
    if (root == NULL) {
        free(freq_table);
        free(pq->nodes);
        free(pq);
        fclose(file);
        return EXIT_FAILURE;
    }
    if (DEBUG) print_heap(pq, "tree");

    print_tree(root, 0);

    free(freq_table);
    free_tree(root);
    free(pq->nodes);
    free(pq);
    fclose(file);
    return 0;
}

FILE* open_file(const char* path, const char* mode) {
    FILE* file = fopen(path, mode);
    if (file == NULL) {
        fprintf(stderr, "[ERROR]: open_file() {} -> Unable to open '%s'!\n", path);
        return NULL;
    }
    return file;
}


size_t* count_run(FILE* file) {
    size_t* frequency_table = malloc(FREQUENCY_TABLE_SIZE * sizeof(size_t));
    if (frequency_table == NULL) {
        fprintf(stderr, "[ERROR]: count_run() {} -> Unable to allocate memory for frequency table!\n");
        return NULL;
    }
    memset(frequency_table, 0, FREQUENCY_TABLE_SIZE * sizeof(size_t)); // set every value to zero, in order to start counting occurance

    char* read_buffer = malloc(READ_BUFFER_SIZE * sizeof(unsigned char));
    if (read_buffer == NULL) {
        fprintf(stderr, "[ERROR]: count_run() {} -> Unable to allocate memory for read buffer!\n");
        return NULL;
    }

    size_t read_bytes;
    while ( (read_bytes = fread(read_buffer, sizeof(unsigned char), READ_BUFFER_SIZE, file)) != 0) {
        for (size_t i = 0; i < read_bytes; i++) {
            frequency_table[(unsigned char) read_buffer[i]]++;
        }
    }

    free(read_buffer);
    return frequency_table;
}


void swap(Node* p1, Node* p2) {
    Node temp = *p1;
    *p1 = *p2;
    *p2 = temp;
}

Heap* create_priority_queue(size_t* list) {
    size_t heap_size = 0;
    for (size_t i = 0; i < FREQUENCY_TABLE_SIZE; i++) {
        if (list[i] != 0) heap_size++;
    }

    Heap* priority_queue = malloc(sizeof(Heap));
    if (priority_queue == NULL) {
        fprintf(stderr, "[ERROR]: create_priority_queue() {} -> Unable to allocate memory for priority_queue heap!\n");
        return NULL;
    }

    priority_queue->nodes = malloc(heap_size * sizeof(Node));
    if (priority_queue->nodes == NULL) {
        fprintf(stderr, "[ERROR]: create_priority_queue() {} -> Unable to allocate memory for priority_queue nodes!\n");
        free(priority_queue);
        return NULL;
    }

    priority_queue->size = 0;
    priority_queue->max_size = heap_size;


    for (size_t i = 0; i < FREQUENCY_TABLE_SIZE; i++) {
        if (list[i] != 0) {
            Node node = { .symbol = (unsigned char)i, .frequency = list[i], .l_node = NULL, .r_node = NULL };
            ssize_t node_index = heap_insert(priority_queue, &node);
            if (node_index == -1) {
                fprintf(stderr, "[ERROR]: create_priority_queue() {} -> Heap insert failed!\n");
                free(priority_queue->nodes);
                free(priority_queue);
                return NULL;
            }
        }
    }

    return priority_queue;
}

ssize_t heap_insert(Heap* heap, Node* node) {
    if (heap->size >= heap->max_size) {
        return -1;
    }

    heap->nodes[heap->size++] = *node;
    size_t node_index = sort_heap_node(heap, heap->size - 1);
    return node_index;
}

ssize_t sort_heap_node(Heap* heap, size_t index) {
    if (index == 0) {
        return 0;
    }
    size_t parent_index = (index - 1) / 2;
    size_t new_index = index;

    if ((heap->nodes[index].frequency < heap->nodes[parent_index].frequency) 
        || (heap->nodes[index].frequency == heap->nodes[parent_index].frequency && heap->nodes[index].symbol < heap->nodes[parent_index].symbol)) {
        swap(&heap->nodes[index], &heap->nodes[parent_index]);
        new_index = sort_heap_node(heap, new_index);
    }
    return new_index;
}

Node* heap_extract(Heap* heap) {
    Node* node = malloc(sizeof(Node));
    if (node == NULL) {
        fprintf(stderr, "[ERROR]: heap_extract() {} -> Unable to allocate memory for the node!\n");
        return NULL;
    }

    if (heap->size == 0) {
        fprintf(stderr, "[ERROR]: heap_extract() {} -> Heap is empty!\n");
        return NULL;
    }
    
    *node = heap->nodes[0];
    heap->nodes[0] = heap->nodes[--heap->size];

    if (DEBUG) print_heap(heap, "extract b4 sort");
    sort_heap(heap, 0);
    if (DEBUG) print_heap(heap, "extract af sort");

    return node;
}

size_t sort_heap(Heap* heap, size_t index) {
    size_t left_child = index * 2 + 1;
    size_t right_child = index * 2 + 2;
    size_t min_index = index;

    if (left_child < heap->size && 
        (heap->nodes[left_child].frequency < heap->nodes[min_index].frequency || (heap->nodes[left_child].frequency == heap->nodes[min_index].frequency 
        && heap->nodes[left_child].symbol < heap->nodes[min_index].symbol))) {
        min_index = left_child;
    }
    if (right_child < heap->size &&
        (heap->nodes[right_child].frequency < heap->nodes[min_index].frequency || (heap->nodes[right_child].frequency == heap->nodes[min_index].frequency 
        && heap->nodes[right_child].symbol < heap->nodes[min_index].symbol))) {
        min_index = right_child;
    }

    if (min_index != index) {
        swap(&heap->nodes[index], &heap->nodes[min_index]);
        if (DEBUG) print_heap(heap, "sort heap"); 
        min_index = sort_heap(heap, min_index);
    }
    return min_index;
}

Node* combine_nodes(Node* n1, Node* n2) {
    Node* new_node = malloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "[ERROR]: combine_nodes() {} -> Unable to combine two nodes!\n");
        return NULL;
    }

    new_node->symbol = 0xFF;
    new_node->frequency = n1->frequency + n2->frequency;
    new_node->l_node = n1;
    new_node->r_node = n2;

    return new_node;
}

Node* build_heap_tree(Heap* heap) {
    while (heap->size >= 2) {
        Node* n1 = heap_extract(heap);
        Node* n2 = heap_extract(heap);
        if (n1 == NULL || n2 == NULL) {
            return NULL;
        }
        if (DEBUG) printf("[combine]: %c (%zu) & %c (%zu)", n1->symbol, n1->frequency, n2->symbol, n2->frequency);

        Node* new_node = combine_nodes(n1, n2);
        if (new_node == NULL) {
            free(n1);
            free(n2);
            return NULL;
        }
        if (DEBUG) printf("-> %X (%zu)\n", new_node->symbol, new_node->frequency);

        ssize_t index = heap_insert(heap, new_node);
        if (index == -1) {
            free(n1);
            free(n2);
            free(new_node);
            return NULL;
        }

        free(new_node);
    }

    if (heap->size == 1) {
        Node* root = malloc(sizeof(Node));
        if (root == NULL) {
            fprintf(stderr, "[ERROR]: build_heap_tree() {} -> Unable to allocate memory for root!\n");
            return NULL;
        }
        *root = heap->nodes[0]; // Copy the last node
        return root;
    }

    return NULL;
}

void print_tree(Node* root, int indent) {
    printf("%*s[%X (%c): (%zu)] ->\n", indent, " ", root->symbol, root->symbol, root->frequency);

    if (root->r_node == NULL && root->l_node == NULL) {
        return;
    } 

    indent += 5;
    print_tree(root->r_node, indent);
    print_tree(root->l_node, indent);
}


void free_tree(Node* root) {
    if (root == NULL) {
        return; // Base case: nothing to free
    }
    // Recursively free left and right subtrees
    free_tree(root->r_node);
    free_tree(root->l_node);
    // Free the current node
    if (DEBUG) printf("[free] %X: (%c)#%zu", root->symbol, root->symbol, root->frequency);
    free(root);
    if (DEBUG) printf(" [DONE!]\n");
}
