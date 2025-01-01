#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static int errors = 0;

void fatal_error(const char* fmt, ...) {

    va_list args;

    fprintf(stderr, "FATAL: ");

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);

    exit(1);
}

void syntax_error(const char* file, int line, const char* fmt, ...) {

    va_list args;

    fprintf(stderr, "ERROR: %s: %d: ", file, line);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
    errors++;
}

void misc_error(const char* fmt, ...) {

    va_list args;

    fprintf(stderr, "ERROR: ");

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
    errors++;
}


int get_errors(void) {
    return errors;
}
