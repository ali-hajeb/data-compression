#include "../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
* Function err
* ------------
*  Prints an error message
*
*  func_name: Name of the parent function
*  message: Error message text
*/
void err(const char* func_name, const char* message) {
    fprintf(stderr, "\n[ERROR]: %s() {} -> %s\n", func_name, message);
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
        fprintf(stderr, "\n[ERROR]: open_file() {} -> Unable to open '%s'!\n", path);
        return NULL;
    }
    return file;
}

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
int extract_filename_format(const char *filepath, char **filename, char **fileformat) {
	int result = -1;
	const char *p = filepath;
	ssize_t last_slash_pos = -1;
	ssize_t last_dot_pos = -1;

	// Finding last slash and last dot
	size_t pos = 0;
	while (*p != '\0') {
		if (*p == '/' || *p == '\\') {
			last_slash_pos = pos;
		}
		if (*p == '.') {
			last_dot_pos = pos;
		}

		pos++;
		p++;
	}

	// Checking if last slash was found
	if (last_slash_pos == -1) {
		printf("\n[ERROR]: extract_filename_format() {} -> Invalid file path! (All file path must contain '/' or '\\'.)\n");
		printf("EXAMPLES:\n\t- C:\\...\\[file]\n\t- ./[file]\n\r");
		return -1;
	}
	// Checking if last dot was found
	if (last_dot_pos < 1) {
		// Assigning filepath end position to the last_dot_pos
		last_dot_pos = pos;
	}

	// Checking if file name has one or more characters. (Some files might be like '.ext')
	size_t filename_size = last_dot_pos - last_slash_pos - 1;
	if (filename_size >= 1) {
		*filename = malloc(filename_size + 1);
		if (*filename == NULL) {
			printf("\n[ERROR]: extract_filename_format() {} -> Unable to allocate memory for filename!\n");
			return -1;
		}

		// Getting file name
		for (size_t i = 0; i < filename_size; i++) {
			(*filename)[i] = filepath[i + last_slash_pos + 1];
		}
		(*filename)[filename_size] = '\0';
		// printf("%s\n", *filename);
		// Setting result = 0
		result = 0;
	}

	// Checking if file has an extension
	if (last_dot_pos > -1 && pos - last_dot_pos > 1) {
		size_t fileformat_size = pos - last_dot_pos - 1;
		*fileformat = malloc(fileformat_size + 1);
		if (*fileformat == NULL) {
			printf("\n[ERROR]: extract_filename_format() {} -> Unable to allocate memory for fileformat!\n");
			return -1;
		}

		// Getting file format
		for (size_t i = 0; i < fileformat_size; i++) {
			(*fileformat)[i] = filepath[i + last_dot_pos + 1];
		}
		(*fileformat)[fileformat_size] = '\0';
		// printf("%s\n", *fileformat);

		// Setting the appropriate result
		// If file name was found:
		if (result == 0) {
			result = 2;
		}
		// If file only has extension
		else {
			result = 1;
		}
	}

	return result;
}

/*
* Function: get_file_size
* -----------------------
*  Returns the size of the file
*
*  file: Pointer to the file
*
*  returns: file size
*/
size_t get_file_size(FILE* file) {
    size_t current_pos = ftell(file);
    fseek(file, 0, SEEK_END);
    size_t end = ftell(file); 
    fseek(file, current_pos, SEEK_SET);
    return end;
}

