#ifndef RESOURCES_H
#define RESOURCES_H
#include <stdlib.h>

typedef struct {
    void** pointers;
    size_t size;
    size_t capacity;
} Resources;

/*
* Function resource_init
* ----------------------
*  Initialize a Resources object to hold future pointers
*
*  initial_capacity: Predicted size for the poiters list (can be zero)
*
*  returns: Resources object
*/
Resources resources_init(size_t initial_capacity);

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
int resources_add(Resources* res, void* ptr);

/*
* Function: resources_cleanup
* ---------------------------
*  Iterates through the pointers list and frees them if they
*  are not NULL
*
*  resource: Pointer to the Resources Object
*/
void resources_cleanup(Resources* res);
#endif
