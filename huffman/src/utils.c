#include <stdio.h>

/*
* Function err
* ------------
*  Prints an error message
*
*  func_name: Name of the parent function
*  message: Error message text
*/
void err(const char* func_name, const char* message) {
    fprintf(stderr, "[ERROR]: %s() {} -> %s\n", func_name, message);
}

/*
* Function open_file
* ------------------
*  Returns a file pointer
*
*  path: File path
*  mode: fopen modes
*
*  returns: Pointer to the file. If failed, returns NULL
*/
FILE* open_file(const char* path, const char* mode) {
    FILE* file = fopen(path, mode);
    if (file == NULL) {
        fprintf(stderr, "[ERROR]: open_file() {} -> Unable to open '%s'!\n", path);
        return NULL;
    }
    return file;
}

