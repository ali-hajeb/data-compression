#include "../include/resources.h"
#include <stddef.h>
#include <stdlib.h>

Resources resources_init(size_t initial_capacity) {
    Resources resource = {NULL, 0, 0};

    if (initial_capacity == 0) {
        return resource;
    }

    resource.pointers = malloc(initial_capacity * sizeof(void*));
    if (resource.pointers) {
        resource.capacity = initial_capacity;
        resource.size = 0;
    }
    return resource;
}

int resources_add(Resources* resource, void* new_pointer) {
    if (!resource) {
        return 0;
    }

    if (resource->size >= resource->capacity) {
        size_t new_capacity = resource->capacity == 0 ? 4 : resource->capacity * 2;
        void** new_pointers = realloc(resource->pointers, new_capacity);
        if (new_pointers == NULL) {
            return 0;
        }
        resource->pointers = new_pointers;
        resource->capacity = new_capacity;
    }

    resource->pointers[(resource->size)++] = new_pointer;
    return 1;
}

void resources_cleanup(Resources* resource) {
    if (resource == NULL || resource->pointers == NULL) {
        return;
    }

    for (size_t i = 0; i < resource->size; i++) {
        if (resource->pointers[i] != NULL) {
            free(resource->pointers[i]);
            resource->pointers[i] = NULL;
        }
    }

    free(resource->pointers);
    resource->size = 0;
    resource->capacity = 0;
}
