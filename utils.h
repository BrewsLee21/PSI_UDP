#include <stdio.h>
#include <stdint.h>

#ifndef UTILS_H
#define UTILS_H

char *get_filename(char* path);
uint32_t get_file_size(FILE *stream);

#endif
