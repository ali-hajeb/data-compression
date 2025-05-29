#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

FILE* open_file(const char* path, const char* mode);
size_t* count_run(FILE* file);
ssize_t sort(Node* list, size_t size);
void swap(Node* p1, Node* p2);
Node* create_priority_queue(size_t* list);
Node* combine_nodes(Node* n1, Node* n2);
Node* build_tree(Node* list, size_t last_index);
void print_tree(Node* tree, int indent); 
void output_file();

void encode();
void decode();

int main(void) {
    FILE* file = open_file("./pic.bmp", "rb");
    if (file == NULL) return EXIT_FAILURE;
    
    size_t* freq_table = count_run(file);
    if (freq_table == NULL) return EXIT_FAILURE;

    Node* pq = create_priority_queue(freq_table);
    if (pq == NULL) return EXIT_FAILURE;

    size_t last_index = sort(pq, FREQUENCY_TABLE_SIZE);
    printf("last index: %zu\n", last_index);

    Node* tree = build_tree(pq, last_index);
    if (tree == NULL) return EXIT_FAILURE;
    printf("root: %d\n", tree->frequency);

    print_tree(tree, 0);

    fclose(file);
    free(freq_table);
    free(pq);
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
    // for (size_t i = 0; i < FREQUENCY_TABLE_SIZE; i++) {
    //     printf("[%llX]: %zu\n", i, frequency_table[i]);
    // }
    // printf("----------------------\n");

    char* read_buffer = malloc(READ_BUFFER_SIZE * sizeof(unsigned char));
    if (read_buffer == NULL) {
        fprintf(stderr, "[ERROR]: count_run() {} -> Unable to allocate memory for read buffer!\n");
        return NULL;
    }

    size_t read_bytes;
    while ( (read_bytes = fread(read_buffer, sizeof(unsigned char), READ_BUFFER_SIZE, file)) != 0) {
        for (size_t i = 0; i < read_bytes; i++) {
            frequency_table[read_buffer[i]]++;
        }
    }

    free(read_buffer);
    return frequency_table;
}

ssize_t sort(Node* list, size_t size) {
    size_t last_index = -1;
    for (size_t i = 0; i < size; i++) {
        for (size_t j = i + 1; j < size; j++) {
            if ((list[i].frequency < list[j].frequency)
                || (list[i].frequency == list[j].frequency && list[i].symbol < list[j].symbol)) {
                swap(&list[i] , &list[j]); 
            }
        }
    }


    size_t max_count = list[0].frequency;
    if (max_count == 0) max_count = 1;
    max_count = max_count / 255 + 1; // scale down the counts so it can be fit into unsigned char

    for (size_t i = 0; i < size; i++) {
        if (list[i].frequency == 0) break;

        size_t scaled_count = list[i].frequency / max_count;
        if (list[i].frequency != 0 && scaled_count  == 0) {
            scaled_count = 1;
        }
        list[i].frequency = scaled_count;

        // printf("[%X]: %c (%zu)\n", list[i].symbol, list[i].symbol, list[i].frequency);
        last_index++;
    }

    return last_index;
}

void swap(Node* p1, Node* p2) {
    Node temp = *p1;
    *p1 = *p2;
    *p2 = temp;
}

Heap* create_priority_queue(size_t* list) {
    // Node* priority_queue = malloc(FREQUENCY_TABLE_SIZE * sizeof(Node));
    // if (priority_queue == NULL) {
    //     fprintf(stderr, "[ERROR]: create_priority_queue() {} -> Unable to allocate memory for priority queue!\n");
    //     return NULL;
    // }
    //
    // for (size_t i = 0; i < FREQUENCY_TABLE_SIZE; i++) {
    //     priority_queue[i].symbol = i;
    //     priority_queue[i].frequency = list[i];
    //     priority_queue[i].r_node = NULL;
    //     priority_queue[i].l_node = NULL;
    // }
    //
    // return priority_queue;
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
        return NULL;
    }

    priority_queue->size = 0;
    priority_queue->max_size = heap_size;


    for (size_t i = 0; i < FREQUENCY_TABLE_SIZE; i++) {
        if (list[i] != 0) {
            priority_queue->nodes[i].symbol = i;
            priority_queue->nodes[i].frequency = list[i];
            priority_queue->nodes[i].r_node = NULL;
            priority_queue->nodes[i].l_node = NULL;
        }
    }

    return priority_queue;
}

void min_heapify(Heap* heap, size_t index) {
    size_t left_child = index * 2 + 1;
    size_t right_child = index * 2 + 2;
    size_t min_index = index;

    if (left_child >= heap->size || left_child < 0) left_child = -1;
    if (right_child >= heap->size || right_child < 0) right_child = -1;

    if (left_child > -1 && heap->nodes[left_child].frequency < heap->nodes[index].frequency) {
        min_index = left_child;
    }
    if (right_child > -1 && heap->nodes[right_child].frequency < heap->nodes[index].frequency) {
        min_index = right_child;
    }

    if (min_index != index) {
        swap(&heap->nodes[index], &heap->nodes[min_index]);
        min_heapify(heap, min_index);
    }
}

Node* combine_nodes(Node* n1, Node* n2) {
    Node* new_node = malloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "[ERROR]: combine_nodes() {} -> Unable to combine two nodes!\n");
        return NULL;
    }

    // printf("1\n");
    new_node->symbol = 0xFF;
    // printf("2\n");
    new_node->frequency = n1->frequency + n2->frequency;
    // printf("3\n");
    new_node->l_node = n1;
    // printf("4\n");
    new_node->r_node = n2;

    // printf("\t-> |%X (%c): %zu\t", new_node->symbol, new_node->symbol, new_node->frequency);

    return new_node;
}

Node* build_tree(Node* priority_queue, size_t last_index) {
    // printf("[combine]: %c (%zu) & %c (%zu)", priority_queue[last_index].symbol, priority_queue[last_index].frequency, priority_queue[last_index - 1].symbol, priority_queue[last_index - 1].frequency);
    Node* last_node = combine_nodes(&priority_queue[last_index], &priority_queue[last_index - 1]);
    // printf("-> %X (%zu)\n", last_node->symbol, last_node->frequency);

    for (ssize_t i = last_index - 2; i >= 0; i--) {
        // printf("[combine]: %c (%zu) & %c (%zu) ", priority_queue[i].symbol, priority_queue[i].frequency, last_node->symbol, last_node->frequency);
        last_node = combine_nodes(&priority_queue[i], last_node);
        // printf("-> %X (%zu)\n", last_node->symbol, last_node->frequency);
    }

    return last_node;
}

void print_tree(Node* tree, int indent) {
    if (tree == NULL) {
        printf("%*c NULL\n", indent, ' ');
        return;
    }
    printf("%*X (%c): [%zu] ->\n", indent, tree->symbol, tree->symbol, tree->frequency);
    indent += 5;
    print_tree(tree->r_node, indent);
    print_tree(tree->l_node, indent);
    // free(tree);
}
