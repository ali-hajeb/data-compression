#include "../include/resources.h"

#include <stdlib.h>

/*
* Function resource_init
* ----------------------
*  Initialize a Resources object to hold future pointers
*
*  initial_capacity: Predicted size for the poiters list (can be zero)
*
*  returns: Resources object
*/
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

/*
* Function: resources_add
* -----------------------
*  Adds the pointer to the pointers list
*
*  resource: Pointer to the Resources object
*  new_pointer: Pointer to be added
*
*  returns: If failed (0), On success (1)
*/
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

/*
* Function: resources_cleanup
* ---------------------------
*  Iterates through the pointers list and frees them if they
*  are not NULL
*
*  resource: Pointer to the Resources Object
*/
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
