#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "utils.h"

char *get_filename(char* path) {
    char *filename = strrchr(path, '/');
    return filename ? filename + 1 : path;
}

uint32_t get_file_size(FILE *stream) {
    fseek(stream, 0, SEEK_END);
    long size = ftell(stream);
    rewind(stream);

    return (uint32_t)size;
}



