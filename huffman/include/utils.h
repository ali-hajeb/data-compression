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

/*
* Function: extract_filename_format
* ---------------------------------
*  Extracts filename and extention from file path
*  
*  filepath: Pointer to the file path
*  filename: Pointer to the filename char_pointer
*  fileformat: Pointer to the fileformat char_pointer
*
*  returns: If failed (-1), Only file name (1), Only extention (2), both filename & extention (3)
*/
int extract_filename_format(const char *filepath, char **filename, char **fileformat);

/*
* Function: get_file_size
* -----------------------
*  Returns the size of the file
*
*  file: Pointer to the file
*
*  returns: file size
*/
size_t get_file_size(FILE* file);
#endif
