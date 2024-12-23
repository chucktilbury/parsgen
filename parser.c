
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "parser.h"
#include "scanner.h"
#include "pointer_list.h"

static ast_grammar_t* parse_grammar(parser_state_t* pstate);
static ast_rule_t* parse_rule(parser_state_t* pstate);
static ast_terminal_t* parse_terminal(parser_state_t* pstate);
static ast_rule_element_t* parse_rule_element(parser_state_t* pstate);
static ast_rule_element_list_t* parse_rule_element_list(parser_state_t* pstate);
static ast_rule_function_t* parse_rule_function(parser_state_t* pstate);
static ast_one_or_more_func_t* parse_one_or_more_func(parser_state_t* pstate);
static ast_zero_or_one_func_t* parse_zero_or_one_func(parser_state_t* pstate);
static ast_zero_or_more_func_t* parse_zero_or_more_func(parser_state_t* pstate);
static ast_or_func_t* parse_or_func(parser_state_t* pstate);

/*
 * grammar {
 *    rule+
 * }
 */
static ast_grammar_t* parse_grammar(parser_state_t* pstate) {

    for(token_t* tok = get_token(); tok->type != END_OF_INPUT; tok = advance_token())
        printf("%d %s = \"%s\" -> %s\n", tok->line_no,
            tok->name, tok->text, tok_type_to_str(tok));

    return NULL;
}

/*
 * rule {
 *    NON_TERMINAL OCURLY rule_function CCURLY
 * }
 */
static ast_rule_t* parse_rule(parser_state_t* pstate) {

    return NULL;
}

/*
 * terminal {
 *     TERMINAL_NAME |
 *     TERMINAL_OPER |
 *     TERMINAL_SYMBOL
 * }
 */
static ast_terminal_t* parse_terminal(parser_state_t* pstate) {

    return NULL;
}

/*
 * rule_element {
 *     NON_TERMINAL |
 *     terminal |
 *     rule_function
 * }
 */
static ast_rule_element_t* parse_rule_element(parser_state_t* pstate) {

    return NULL;
}

/*
 * rule_element_list {
 *     OPAREN rule_element+ CPAREN |
 *     rule_element+
 * }
 */
static ast_rule_element_list_t* parse_rule_element_list(parser_state_t* pstate) {

    return NULL;
}

/*
 * rule_function {
 *     or_func |
 *     zero_or_more_func |
 *     zero_or_one_func |
 *     one_or_more_func
 * }
 */
static ast_rule_function_t* parse_rule_function(parser_state_t* pstate) {

    return NULL;
}

/*
 * one_or_more_func {
 *     rule_element_list ONE_OR_MORE
 * }
 */
static ast_one_or_more_func_t* parse_one_or_more_func(parser_state_t* pstate) {

    return NULL;
}

/*
 * zero_or_one_func {
 *     rule_element_list ZERO_OR_ONE
 * }
 */
static ast_zero_or_one_func_t* parse_zero_or_one_func(parser_state_t* pstate) {

    return NULL;
}

/*
 * zero_or_more_func {
 *     rule_element_list ZERO_OR_MORE
 * }
 */
static ast_zero_or_more_func_t* parse_zero_or_more_func(parser_state_t* pstate) {

    return NULL;
}

/*
 * or_func {
 *     rule_element PIPE rule_element
 * }
 */
static ast_or_func_t* parse_or_func(parser_state_t* pstate) {

    return NULL;
}

/*
 * Main interface into the parser.
 */

parser_state_t* init_parser(const char* file_name) {

    assert(file_name != NULL);

    init_scanner(file_name);
    parser_state_t* ptr = malloc(sizeof(parser_state_t));
    ptr->state = 0;

    return ptr;
}

ast_grammar_t* parse(parser_state_t* ptr) {

    return parse_grammar(ptr);
}
