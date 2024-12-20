
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "parser.h"
#include "scanner.h"

void parse(void) {

    for(token_t* tok = get_token(); tok->type != END_OF_INPUT; tok = advance_token())
        printf("%d %s = \"%s\" -> %s\n", tok->line_no,
            tok->name, tok->text, tok_type_to_str(tok));
}

void rule(void) {
}
