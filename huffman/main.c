#include <limits.h>
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
    uint32_t code;
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

void generate_huffman_code(Code* code_table, uint32_t code, uint8_t depth, Node* node);
void write_bits(Bitwriter* bit_writer, uint32_t code, uint8_t length);
Bitwriter* init_bitwriter(FILE* file);
FILE* open_file(const char* path, const char* mode);
void output_file();

void encode(FILE* input_file, Bitwriter* bit_writer, Code* code_table);
int compress(FILE* input_file, FILE* output_file, Bitwriter* bit_writer, Code* code_table, size_t* freq_table);
void decompress(FILE* input_file, FILE* output_file);
void decode();

int main(void) {
    // FILE* file = open_file("./pic.bmp", "rb");
    FILE* file = open_file("./test", "rb");
    if (file == NULL) return EXIT_FAILURE;
    
    FILE* output_file = open_file("./test.huff", "wb");
    if (output_file == NULL) return EXIT_FAILURE;

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

    Node* root = build_tree(pq);
    if (root == NULL) {
        free(freq_table);
        free(pq->nodes);
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

    Bitwriter* bit_writer = init_bitwriter(output_file);
    compress(file, output_file, bit_writer, code_table, freq_table);

    fclose(output_file);
    FILE* dec = fopen("dec", "wb");
    FILE* com = fopen("./test.huff", "rb");
    decompress(com, dec);
    
    fclose(dec);
    fclose(com);

    free(freq_table);
    free_tree(root);
    free(pq->nodes);
    free(pq);
    free(bit_writer);
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
    size_t max_count = 0;
    for (size_t i = 0; i < FREQUENCY_TABLE_SIZE; i++) {
        if (list[i] != 0) {
            heap_size++;
            if (list[i] > max_count) {
                max_count = list[i];
            }
        }
    }
    max_count /= 255;
    max_count++;

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
            size_t frequency = list[i] / max_count;
            if (frequency == 0) frequency = 1;
            Node node = { .symbol = (unsigned char)i, .frequency = frequency, .l_node = NULL, .r_node = NULL };
            printf("%2X --->> %zu %zu\n", i, list[i], frequency);
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

    sort_heap(heap, 0);

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
    // printf("%3d\t[free] %p %X: (%c)#%zu", free_counter++, root, root->symbol, root->symbol, root->frequency);
    free(root);
    // printf(" [DONE!]\n");
}


void generate_huffman_code(Code* code_table, uint32_t code, uint8_t depth, Node* node) {
    if (node == NULL) return;
    if (node->l_node == NULL && node->r_node == NULL) {
        code_table[node->symbol].code = code; 
        code_table[node->symbol].length = depth; 
        printf("%2x -> ", node->symbol);
        printbits_n(code_table[node->symbol].code, 32);
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

Bitwriter* init_bitwriter(FILE* file) {
    Bitwriter* bit_writer = malloc(sizeof(Bitwriter));
    if (bit_writer == NULL) {
        fprintf(stderr, "[ERROR]: init_bitwriter() {} -> Unable to allocate memory for bitwriter!\n");
        return NULL;
    }
    bit_writer->file = file;
    bit_writer->bit_count = 0;
    memset(bit_writer->buffer, 0, OUTPUT_BUFFER_SIZE);
    return bit_writer;
}

void write_bits(Bitwriter* writer, uint32_t code, uint8_t length) {
    if (length == 0) return;
    for (uint8_t i = 0; i < length; i++) {
        size_t byte_idx = writer->bit_count / 8;
        size_t bit_idx = writer->bit_count % 8;
        if (byte_idx >= OUTPUT_BUFFER_SIZE) {
            fwrite(writer->buffer, 1, OUTPUT_BUFFER_SIZE, writer->file);
            memset(writer->buffer, 0, OUTPUT_BUFFER_SIZE);
            writer->bit_count = 0;
            byte_idx = 0;
            bit_idx = 0;
        }
        // Extract bit from code (MSB first)
        int bit = (code >> (length - 1 - i)) & 1;
        // Set bit in buffer
        writer->buffer[byte_idx] |= (bit << (7 - bit_idx));
        writer->bit_count++;
    }
}

void flush_bits(Bitwriter* bit_writer) {
    size_t bytes = (bit_writer->bit_count + 7) / 8;
    if (bytes > 0) {
        printf("here %zu\n", bytes);
        printbits_n(bit_writer->buffer[0], 32);
        printf("%s\n", bit_writer->buffer);
        printbits_n(bit_writer->buffer[1], 32);
        printf("%s\n", bit_writer->buffer);
        fwrite(bit_writer->buffer, 1, bytes, bit_writer->file);
    }
    memset(bit_writer->buffer, 0, OUTPUT_BUFFER_SIZE);
    bit_writer->bit_count = 0;
}

void encode(FILE* input_file, Bitwriter* bit_writer, Code* code_table) {
    unsigned char read_buffer[OUTPUT_BUFFER_SIZE];
    size_t bytes_read = 0;
    fseek(input_file, 0, SEEK_SET);

    while((bytes_read = fread(read_buffer, 1, OUTPUT_BUFFER_SIZE, input_file)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            unsigned char symbol = read_buffer[i];
            if (code_table[symbol].length > 0) {
                write_bits(bit_writer, code_table[symbol].code, code_table[symbol].length);
            }
        }
    }
    flush_bits(bit_writer);
}

int compress(FILE* input_file, FILE* output_file, Bitwriter* bit_writer, Code* code_table, size_t* freq_table) {
    unsigned char non_zero = 0;
    size_t max_count = 0;
    for (size_t i = 0; i < FREQUENCY_TABLE_SIZE; i++) {
        if (freq_table[i] > 0) {
            non_zero++;
            if (freq_table[i] > max_count) {
                max_count = freq_table[i];
            }
        }
    }
    fwrite(&non_zero, 1, 1, output_file);

    max_count /= 255;
    max_count++;
    for (size_t i = 0; i < FREQUENCY_TABLE_SIZE; i++) {
        if (freq_table[i] > 0) {
            unsigned char symbol = (unsigned char) i;
            fwrite(&symbol, 1, 1, output_file);
            unsigned char count = freq_table[i] / max_count;
            if (count == 0) count++;
            fwrite(&count, 1, 1, output_file);
        }
    }

    printf("DONE table\n");
    encode(input_file, bit_writer, code_table);
    return 1;
}

void decompress(FILE* input_file, FILE* output_file) {
    printf("====================================================\n");
    size_t frequency_table[FREQUENCY_TABLE_SIZE];
    memset(&frequency_table, 0, FREQUENCY_TABLE_SIZE * sizeof(size_t));
    unsigned char char_count = '0';
    fseek(input_file, 0, SEEK_SET);
    size_t read_bytes = fread(&char_count, sizeof(unsigned char), 1, input_file);
    if (read_bytes <= 0) {
        fprintf(stderr, "[ERROR]: decompress() {} -> File is not valid!\n");
        return;
    }
    printf("%d\n", char_count);

    unsigned char* buffer = malloc(char_count * sizeof(unsigned char) * 2);
    if (buffer == NULL) {
        fprintf(stderr, "[ERROR]: decompress() {} -> Unable to allocate memory for buffer!\n");
        return;
    }
    read_bytes = fread(buffer, sizeof(unsigned char), char_count * 2, input_file);

    for (unsigned char i = 0; i < char_count * 2; i += 2) {
        unsigned char symbol = buffer[i];
        unsigned char count = buffer[i + 1];
        frequency_table[symbol] = count;
        printf("[%2X] -> %d\n", symbol, frequency_table[symbol]);
    }
    free(buffer);

    Heap* pq = create_priority_queue(frequency_table);
    if (pq == NULL) {
        return ;
    }

    if (DEBUG) print_heap(pq, "priority_queue");

    Node* root = build_tree(pq);
    if (root == NULL) {
        free(pq->nodes);
        free(pq);
        return ;
    }
    if (DEBUG) print_heap(pq, "tree");

    print_tree(root, 0);

    read_bytes = 0;
    unsigned char* read_buffer = malloc(READ_BUFFER_SIZE);
    if (read_buffer == NULL) {
        fprintf(stderr, "[ERROR]: decompress() {} -> Unable to allocate memory for read_buffer!\n");
        return;
    }

    Node node = *root;
    while ((read_bytes = fread(read_buffer, 1, READ_BUFFER_SIZE, input_file)) > 0) {
        for (size_t i = 0; i < read_bytes; i++) {
            unsigned char byte = read_buffer[i];
            printf("BYTE: ");
            printbits_n(byte, 8);
            printf("\n");
            for (int j = CHAR_BIT - 1; j >= 0; j--) {
                unsigned char mask = 1 << j;
                printf("\n%d. MASK: ", j);
                printbits_n(mask, 8);
                printf("\n");
                if ((byte & mask) != 0) {
                    node = *node.r_node;
                    printf("1");
                } else {
                    node = *node.l_node;
                    printf("0");
                }

                if (node.l_node == NULL || node.r_node == NULL) {
                    printf("\n%d. SYMBOL [%2X]\n", j, node.symbol);
                    node = *root;
                }
            }
        }
    }
        printf("SYMBOL [%2X]\n", node.symbol);

    free(read_buffer);
}
