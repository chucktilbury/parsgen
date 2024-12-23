
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>

#include "ast.h"

#define PRE_STATE do { \
    if(state->pre != NULL) \
        (*state->pre)((void*)ptr, (void*)state); \
    } while(0)

#define POST_STATE do { \
    if(state->post != NULL) \
        (*state->pre)((void*)ptr, (void*)state); \
    } while(0)

static void traverse_grammar(struct _ast_grammar_t_* ptr, ast_state_t* state);
static void traverse_rule(struct _ast_rule_t_* ptr, ast_state_t* state);
static void traverse_terminal(struct _ast_terminal_t_* ptr, ast_state_t* state);
static void traverse_rule_element(struct _ast_rule_element_t_* ptr, ast_state_t* state);
static void traverse_rule_element_list(struct _ast_rule_element_list_t_* ptr, ast_state_t* state);
static void traverse_rule_function(struct _ast_rule_function_t_* ptr, ast_state_t* state);
static void traverse_one_or_more_func(struct _ast_one_or_more_func_t_* ptr, ast_state_t* state);
static void traverse_zero_or_one_func(struct _ast_zero_or_one_func_t_* ptr, ast_state_t* state);
static void traverse_zero_or_more_func(struct _ast_zero_or_more_func_t_* ptr, ast_state_t* state);
static void traverse_or_func(struct _ast_or_func_t_* ptr, ast_state_t* state);

/*
 * grammar {
 *    rule+
 * }
 */
static void traverse_grammar(struct _ast_grammar_t_* ptr, ast_state_t* state) {

    PRE_STATE;
    POST_STATE;
}

/*
 * rule {
 *    NON_TERMINAL OCURLY rule_function CCURLY
 * }
 */
static void traverse_rule(struct _ast_rule_t_* ptr, ast_state_t* state) {

    PRE_STATE;
    POST_STATE;
}

/*
 * terminal {
 *     TERMINAL_NAME |
 *     TERMINAL_OPER |
 *     TERMINAL_SYMBOL
 * }
 */
static void traverse_terminal(struct _ast_terminal_t_* ptr, ast_state_t* state) {

    PRE_STATE;
    POST_STATE;
}

/*
 * rule_element {
 *     NON_TERMINAL |
 *     terminal |
 *     rule_function
 * }
 */
static void traverse_rule_element(struct _ast_rule_element_t_* ptr, ast_state_t* state) {

    PRE_STATE;
    POST_STATE;
}

/*
 * rule_element_list {
 *     OPAREN rule_element+ CPAREN |
 *     rule_element+
 * }
 */
static void traverse_rule_element_list(struct _ast_rule_element_list_t_* ptr, ast_state_t* state) {

    PRE_STATE;
    POST_STATE;
}

/*
 * rule_function {
 *     or_func |
 *     zero_or_more_func |
 *     zero_or_one_func |
 *     one_or_more_func
 * }
 */
static void traverse_rule_function(struct _ast_rule_function_t_* ptr, ast_state_t* state) {

    PRE_STATE;
    POST_STATE;
}

/*
 * one_or_more_func {
 *     rule_element_list ONE_OR_MORE
 * }
 */
static void traverse_one_or_more_func(struct _ast_one_or_more_func_t_* ptr, ast_state_t* state) {

    PRE_STATE;
    POST_STATE;
}

/*
 * zero_or_one_func {
 *     rule_element_list ZERO_OR_ONE
 * }
 */
static void traverse_zero_or_one_func(struct _ast_zero_or_one_func_t_* ptr, ast_state_t* state) {

    PRE_STATE;
    POST_STATE;
}

/*
 * zero_or_more_func {
 *     rule_element_list ZERO_OR_MORE
 * }
 */
static void traverse_zero_or_more_func(struct _ast_zero_or_more_func_t_* ptr, ast_state_t* state) {

    PRE_STATE;
    POST_STATE;
}

/*
 * or_func {
 *     rule_element PIPE rule_element
 * }
 */
static void traverse_or_func(struct _ast_or_func_t_* ptr, ast_state_t* state) {

    PRE_STATE;
    POST_STATE;
}

static size_t get_ast_node_size(ast_type_t type) {

    return
        (type == AST_GRAMMAR)? sizeof(ast_grammar_t) :
        (type == AST_RULE)? sizeof(ast_rule_t) :
        (type == AST_TERMINAL)? sizeof(ast_terminal_t) :
        (type == AST_RULE_ELEMENT)? sizeof(ast_rule_element_t) :
        (type == AST_RULE_ELEMENT_LIST)? sizeof(ast_rule_element_list_t) :
        (type == AST_RULE_FUNCTION)? sizeof(ast_rule_function_t) :
        (type == AST_ONE_OR_MORE_FUNC)? sizeof(ast_one_or_more_func_t) :
        (type == AST_ZERO_OR_ONE_FUNC)? sizeof(ast_zero_or_one_func_t) :
        (type == AST_ZERO_OR_MORE_FUNC)? sizeof(ast_zero_or_more_func_t) :
        (type == AST_OR_FUNC)? sizeof(ast_or_func_t) : (size_t)-1;

}

ast_node_t* create_ast_node(ast_type_t type) {

    ast_node_t* ptr = malloc(get_ast_node_size(type));
    assert(ptr != NULL);

    ptr->type = type;

    return ptr;
}

ast_type_t get_ast_node_type(ast_node_t* node) {

    return node->type;
}

void traverse_ast(ast_grammar_t* grammar, ast_state_t* state) {

    traverse_grammar(grammar, state);
}
