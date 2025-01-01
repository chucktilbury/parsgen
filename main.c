
// #include <errno.h>
#include <stdio.h>
// #include <string.h>

#include "ast.h"
#include "parser.h"

int main(int argc, char** argv) {

    if(argc < 2) {
        printf("syntax: %s filename\n", argv[0]);
        return 1;
    }

    void* ast = parse(init_parser(argv[1]));

    traverse_ast(ast, NULL);

    uninit_scanner();
    return 0;
}
