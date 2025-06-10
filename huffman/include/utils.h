#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>

/*
* Function err
* ------------
*  Prints an error message
*
*  func_name: Name of the parent function
*  message: Error message text
*/
void err(const char* func_name, const char* message);

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
FILE* open_file(const char* path, const char* mode);
#endif
