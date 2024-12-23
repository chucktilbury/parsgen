
#ifndef _PARSER_H_
#define _PARSER_H_

#include "ast.h"

typedef ast_grammar_t rule_list_t;
typedef struct _parser_state_ {
    int state; // dummy
} parser_state_t;

ast_grammar_t* parse(parser_state_t* ptr);
parser_state_t* init_parser(const char* file_name);

#endif /* _PARSER_H_ */
