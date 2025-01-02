#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "errors.h"
#include "pointer_list.h"

#define PRE_STATE                                        \
    do {                                                 \
        if(state != NULL) {                              \
            if(state->pre != NULL)                       \
                (*state->pre)((void*)ptr, (void*)state); \
        }                                                \
    } while(0)

#define POST_STATE                                       \
    do {                                                 \
        if(state != NULL) {                              \
            if(state->post != NULL)                      \
                (*state->pre)((void*)ptr, (void*)state); \
        }                                                \
    } while(0)

#define TRACE_AST_STATE

#ifdef TRACE_AST_STATE
static int depth = 0;
#define DINC 2
#define TRACE(fmt, ...)                           \
    do {                                          \
        fprintf(stdout, "%*sTRACE: ", depth, ""); \
        fprintf(stdout, fmt, ##__VA_ARGS__);      \
        fprintf(stdout, "\n");                    \
    } while(false)

#define ENTER                                                   \
    do {                                                        \
        fprintf(stdout, "%*sENTER: %s\n", depth, "", __func__); \
        depth += DINC;                                          \
    } while(false)

#define RETURN                                                   \
    do {                                                         \
        depth -= DINC;                                           \
        fprintf(stdout, "%*sRETURN: %s\n", depth, "", __func__); \
        return;                                                  \
    } while(false)

#else
#define TRACE
#define ENTER
#define RETURN
#endif

static void traverse_grammar(ast_grammar_t* ptr, ast_state_t* state);
static void traverse_rule(ast_rule_t* ptr, ast_state_t* state);
static void traverse_rule_element(ast_rule_element_t* ptr, ast_state_t* state);
static void traverse_one_or_more_func(ast_one_or_more_func_t* ptr, ast_state_t* state);
static void traverse_zero_or_one_func(ast_zero_or_one_func_t* ptr, ast_state_t* state);
static void traverse_zero_or_more_func(ast_zero_or_more_func_t* ptr, ast_state_t* state);
static void traverse_or_func(ast_or_func_t* ptr, ast_state_t* state);
static void traverse_group_func(ast_group_func_t* ptr, ast_state_t* state);

/*
 * grammar {
 *     +rule END_OF_INPUT
 * }
 *
 */
static void traverse_grammar(ast_grammar_t* ptr, ast_state_t* state) {
}

/*
 * rule {
 *     NON_TERMINAL '{' +rule_element '}'
 * }
 *
 */
static void traverse_rule(ast_rule_t* ptr, ast_state_t* state) {
}

/*
 * rule_element {
 *     NON_TERMINAL |
 *     TERMINAL_NAME |
 *     TERMINAL_OPER |
 *     TERMINAL_SYMBOL |
 *     or_func |
 *     zero_or_more_func |
 *     zero_or_one_func |
 *     one_or_more_func |
 *     group_func
 * }
 *
 */
static void traverse_rule_element(ast_rule_element_t* ptr, ast_state_t* state) {
}

/*
 * one_or_more_func {
 *     '+' rule_element
 * }
 *
 */
static void traverse_one_or_more_func(ast_one_or_more_func_t* ptr, ast_state_t* state) {
}

/*
 * zero_or_one_func {
 *     '?' rule_element
 * }
 *
 */
static void traverse_zero_or_one_func(ast_zero_or_one_func_t* ptr, ast_state_t* state) {
}

/*
 * zero_or_more_func {
 *     '*' rule_element
 * }
 *
 */
static void traverse_zero_or_more_func(ast_zero_or_more_func_t* ptr, ast_state_t* state) {
}

/*
 * or_func {
 *     '|' rule_element
 * }
 *
 */
static void traverse_or_func(ast_or_func_t* ptr, ast_state_t* state) {
}

/*
 * group_func {
 *     '(' +rule_element ')'
 * }
 *
 */
static void traverse_group_func(ast_group_func_t* ptr, ast_state_t* state) {
}

static size_t get_ast_node_size(ast_type_t type) {

    return (type == AST_GRAMMAR)? sizeof(ast_grammar_t):
        (type == AST_RULE)? sizeof(ast_rule_t):
        (type == AST_RULE_ELEMENT)? sizeof(ast_rule_element_t):
        (type == AST_ONE_OR_MORE_FUNC)? sizeof(ast_one_or_more_func_t):
        (type == AST_ZERO_OR_ONE_FUNC)? sizeof(ast_zero_or_one_func_t):
        (type == AST_ZERO_OR_MORE_FUNC)? sizeof(ast_zero_or_more_func_t):
        (type == AST_OR_FUNC)? sizeof(ast_or_func_t):
        (type == AST_GROUP_FUNC)? sizeof(ast_group_func_t): (size_t)-1;
}

void traverse_ast(void* ptr, void* state) {

    ENTER;
    traverse_grammar((ast_grammar_t*)ptr, (ast_state_t*)state);
    RETURN;
}

ast_type_t get_ast_node_type(void* node) {

    ENTER;
    traverse_grammar((ast_grammar_t*)grammar, (ast_state_t*)state);
    RETURN;
}

ast_node_t* create_ast_node(ast_type_t type) {

    ast_node_t* ptr = _ALLOC(get_ast_node_size(type));
    ptr->type = type;

    return ptr;
}

