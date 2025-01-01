
#include "errors.h"
#include <stdlib.h>
#include <string.h>

void* _mem_alloc(size_t size) {

    void* ptr = malloc(size);
    if(ptr == NULL)
        fatal_error("cannot allocate %lu bytes", size);

    memset(ptr, 0, size);
    return ptr;
}

void* _mem_realloc(void* optr, size_t size) {

    void* nptr = realloc(optr, size);
    if(nptr == NULL)
        fatal_error("cannot re-allocate %lu bytes", size);

    return nptr;
}

void* _mem_copy(void* optr, size_t size) {

    void* nptr = malloc(size);
    if(nptr == NULL)
        fatal_error("cannot allocate to copy %lu bytes", size);

    memcpy(nptr, optr, size);
    return nptr;
}

char* _mem_copy_string(const char* str) {

    size_t len = strlen(str) + 1;
    char* ptr = malloc(len);
    if(ptr == NULL)
        fatal_error("cannot allocate %lu bytes for string", len);

    memcpy(ptr, str, len);
    return ptr;
}

void _mem_free(void* ptr) {

    if(ptr != NULL)
        free(ptr);
}
