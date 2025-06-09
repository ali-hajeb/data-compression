#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>

void err(const char* func_name, const char* message);
FILE* open_file(const char* path, const char* mode);
#endif
