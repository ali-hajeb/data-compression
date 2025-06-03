#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define printbits_n(x,n) for (int i=n;i;i--,putchar('0'|(x>>i)&1))
#define DEBUG 1
#define DEBUG_ 0
#define READ_BUFFER_SIZE 256
#define FREQUENCY_TABLE_SIZE 256
#define OUTPUT_BUFFER_SIZE 1024

static int allocator_count = 1;
static int free_count = 1;
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

typedef struct code {
    uint64_t code;
    uint8_t length;
} Code;

typedef struct bitwriter {
    FILE* file;
    unsigned char buffer[OUTPUT_BUFFER_SIZE];
    size_t bit_count;
} Bitwriter;

void log_allocator(void* ptr, const char* loc) {
    if (DEBUG) printf("[allocate] %d %s: %p\n", allocator_count++, loc, ptr);
}

void log_free(void* ptr, const char* loc) {
    if (DEBUG) printf("[free] %d %s: %p\n", free_count++, loc, ptr);
}

void print_heap(Heap* heap, const char* title) {
    if (DEBUG_) {
        printf("============| %s |=================\n", title);
        for (size_t i = 0; i < heap->size; i++) {
            printf("[%2X]: %c (%zu)\n", heap->nodes[i].symbol, heap->nodes[i].symbol, heap->nodes[i].frequency);
        }
        printf("=============================\n");
    }
}

size_t* count_run(FILE* file);
void swap(Node* p1, Node* p2);

ssize_t heap_insert(Heap* heap, Node* node);
size_t sort_heap(Heap* heap, size_t index); 
ssize_t sort_heap_node(Heap* heap, size_t index);
Heap* create_priority_queue(size_t* list); 
Node* heap_extract(Heap* heap);
Node* combine_nodes(Node* n1, Node* n2);

Node* build_tree(Heap* heap);
void print_tree(Node* root, int indent); 
void free_tree(Node* root);

void generate_huffman_code(Code* code_table, uint64_t code, uint8_t depth, Node* node);
void write_bits(Bitwriter* bitwriter, uint32_t code, uint8_t length);
FILE* open_file(const char* path, const char* mode);
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
        log_free(freq_table, "E cpq");
        free(freq_table);
        fclose(file);
        return EXIT_FAILURE;
    }

    if (DEBUG) print_heap(pq, "priority_queue");

    Node* root = build_tree(pq);
    if (root == NULL) {
        log_free(freq_table, "E bht ft");
        free(freq_table);
        log_free(pq->nodes, "E bht pqn");
        free(pq->nodes);
        log_free(pq, "E bht pq");
        free(pq);
        fclose(file);
        return EXIT_FAILURE;
    }
    if (DEBUG) print_heap(pq, "tree");

    print_tree(root, 0);

    Code* code_table = malloc(FREQUENCY_TABLE_SIZE * sizeof(Code));
    if (code_table == NULL) {
        fprintf(stderr, "[ERROR]: main() {} -> Unable to allocate memory for code table!\n");
        free(freq_table);
        free_tree(root);
        free(pq->nodes); 
        free(pq);
        fclose(file);
        return EXIT_FAILURE;
    }

    generate_huffman_code(code_table, 0, 0, root);

    log_free(freq_table, "main fq");
    free(freq_table);
    free_tree(root);
    log_free(pq->nodes, "main pqn");
    free(pq->nodes);
    log_free(pq, "main pq");
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
    log_allocator(frequency_table, "count run fq");

    char* read_buffer = malloc(READ_BUFFER_SIZE * sizeof(unsigned char));
    if (read_buffer == NULL) {
        fprintf(stderr, "[ERROR]: count_run() {} -> Unable to allocate memory for read buffer!\n");
        return NULL;
    }
    log_allocator(read_buffer, "count run read");

    size_t read_bytes;
    while ( (read_bytes = fread(read_buffer, sizeof(unsigned char), READ_BUFFER_SIZE, file)) != 0) {
        for (size_t i = 0; i < read_bytes; i++) {
            frequency_table[(unsigned char) read_buffer[i]]++;
        }
    }

    log_free(read_buffer, "count run read");
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
    log_allocator(priority_queue, "cpq pq");

    priority_queue->nodes = malloc(heap_size * sizeof(Node));
    if (priority_queue->nodes == NULL) {
        fprintf(stderr, "[ERROR]: create_priority_queue() {} -> Unable to allocate memory for priority_queue nodes!\n");
        log_free(priority_queue, "e cpq pq");
        free(priority_queue);
        return NULL;
    }
    log_allocator(priority_queue->nodes, "cpq pqn");

    priority_queue->size = 0;
    priority_queue->max_size = heap_size;


    for (size_t i = 0; i < FREQUENCY_TABLE_SIZE; i++) {
        if (list[i] != 0) {
            Node node = { .symbol = (unsigned char)i, .frequency = list[i], .l_node = NULL, .r_node = NULL };
            ssize_t node_index = heap_insert(priority_queue, &node);
            if (node_index == -1) {
                fprintf(stderr, "[ERROR]: create_priority_queue() {} -> Heap insert failed!\n");
                log_free(priority_queue->nodes, "e for cpq pqn");
                free(priority_queue->nodes);
                log_free(priority_queue, "e for cpq pq");
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
    log_allocator(node, "he n");

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
    log_allocator(new_node, "cn new");

    new_node->symbol = 0xFF;
    new_node->frequency = n1->frequency + n2->frequency;
    new_node->l_node = n1;
    new_node->r_node = n2;

    return new_node;
}

Node* build_tree(Heap* heap) {
    while (heap->size >= 2) {
        Node* n1 = heap_extract(heap);
        Node* n2 = heap_extract(heap);
        if (n1 == NULL || n2 == NULL) {
            return NULL;
        }

        Node* new_node = combine_nodes(n1, n2);
        if (DEBUG) printf("[combine]: %c (%zu) & %c (%zu)", n1->symbol, n1->frequency, n2->symbol, n2->frequency);
        if (new_node == NULL) {
            log_free(n1, "e bht n1");
            free(n1);
            log_free(n2, "e bht n2");
            free(n2);
            return NULL;
        }
        if (DEBUG) printf("-> %X (%zu)\n", new_node->symbol, new_node->frequency);

        ssize_t index = heap_insert(heap, new_node);
        if (index == -1) {
            log_free(n1, "e index bht n1");
            free(n1);
            log_free(n2, "e index bht n2");
            free(n2);
            log_free(new_node, "e index bht new");
            free(new_node);
            return NULL;
        }
        log_free(new_node, "bht new");
        free(new_node);
    }

    return heap_extract(heap);
}

void print_tree(Node* root, int indent) {
    printf("%*s[%p (%c): (%zu)] ->\n", indent, " ", root, root->symbol, root->frequency);

    if (root->r_node == NULL && root->l_node == NULL) {
        return;
    } 

    indent += 5;
    print_tree(root->r_node, indent);
    print_tree(root->l_node, indent);
}

static int free_counter = 1;
void free_tree(Node* root) {
    if (root == NULL) {
        return; // Base case: nothing to free
    }
    // Recursively free left and right subtrees
    free_tree(root->r_node);
    free_tree(root->l_node);
    // Free the current node
    log_free(root, "freetree root");
    // printf("%3d\t[free] %p %X: (%c)#%zu", free_counter++, root, root->symbol, root->symbol, root->frequency);
    free(root);
    // printf(" [DONE!]\n");
}


void generate_huffman_code(Code* code_table, uint64_t code, uint8_t depth, Node* node) {
    if (node == NULL) return;
    if (node->l_node == NULL && node->r_node == NULL) {
        code_table[node->symbol].code = code; 
        code_table[node->symbol].length = depth; 
        printf("%2x -> ", node->symbol);
        printbits_n(code_table[node->symbol].code, 64);
        printf("\n");
        return;
    }

    if (node->r_node != NULL) {
        generate_huffman_code(code_table, (code << 1) | 1, depth + 1, node->r_node);
    }
    if (node->l_node != NULL) {
        generate_huffman_code(code_table, (code << 1), depth + 1, node->l_node);
    }
}
