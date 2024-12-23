
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "parser.h"

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("syntax: %s filename\n", argv[0]);
        return 1;
    }

    parse(init_parser(argv[1]));

    uninit_scanner();
    return 0;
}
