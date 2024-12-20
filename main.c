
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "parser.h"
#include "scan.gen.h"
#include "scanner.h"

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("syntax: %s filename\n", argv[0]);
        return 1;
    }

    init_scanner(argv[1]);

    parse();

   return 0;
}
