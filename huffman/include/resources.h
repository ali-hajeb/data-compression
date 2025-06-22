#include <stddef.h>
#include <stdlib.h>

typedef struct {
    void** pointers;
    size_t size;
    size_t capacity;
} Resources;

Resources resources_init(size_t initial_capacity);

int resources_add(Resources* res, void* ptr);

void resources_cleanup(Resources* res);
