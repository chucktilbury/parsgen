
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

static void traverse_grammar(struct _ast_grammar_t_* ptr, ast_state_t* state);
static void traverse_rule(struct _ast_rule_t_* ptr, ast_state_t* state);
static void traverse_terminal(struct _ast_terminal_t_* ptr, ast_state_t* state);
static void traverse_rule_element(struct _ast_rule_element_t_* ptr, ast_state_t* state);
static void traverse_rule_element_list(struct _ast_rule_element_list_t_* ptr,
                                       ast_state_t* state);
static void traverse_rule_function(struct _ast_rule_function_t_* ptr, ast_state_t* state);
static void traverse_one_or_more_func(struct _ast_one_or_more_func_t_* ptr,
                                      ast_state_t* state);
static void traverse_zero_or_one_func(struct _ast_zero_or_one_func_t_* ptr,
                                      ast_state_t* state);
static void traverse_zero_or_more_func(struct _ast_zero_or_more_func_t_* ptr,
                                       ast_state_t* state);
static void traverse_or_func(struct _ast_or_func_t_* ptr, ast_state_t* state);

/*
 * grammar {
 *    +rule
 * }
 */
static void traverse_grammar(struct _ast_grammar_t_* ptr, ast_state_t* state) {

    ENTER;
    PRE_STATE;

    ast_rule_t* rule = NULL;
    int post = 0;

    while(NULL != (rule = iterate_pointer_list((ptr->rules), &post)))
        traverse_rule(rule, state);

    POST_STATE;
    RETURN;
}

/*
 * rule {
 *    NON_TERMINAL OCURLY rule_element_list CCURLY
 * }
 */
static void traverse_rule(struct _ast_rule_t_* ptr, ast_state_t* state) {

    ENTER;
    PRE_STATE;

    TRACE("non-terminal: %s:%s", ptr->non_term->text, ptr->non_term->name);
    traverse_rule_element_list(ptr->elem_lst, state);

    POST_STATE;
    RETURN;
}

/*
 * terminal {
 *     TERMINAL_NAME |
 *     TERMINAL_OPER |
 *     TERMINAL_SYMBOL
 * }
 */
static void traverse_terminal(struct _ast_terminal_t_* ptr, ast_state_t* state) {

    ENTER;
    PRE_STATE;

    // no traversal
    // all processing happens in the state handlers.
    TRACE("terminal: %s:%s", ptr->term->text, ptr->term->name);

    POST_STATE;
    RETURN;
}

/*
 * rule_element {
 *     NON_TERMINAL |
 *     terminal |
 *     rule_function
 * }
 */
static void traverse_rule_element(struct _ast_rule_element_t_* ptr, ast_state_t* state) {

    ENTER;
    PRE_STATE;

    if(ptr->term != NULL)
        traverse_terminal(ptr->term, state);
    else if(ptr->function != NULL)
        traverse_rule_function(ptr->function, state);
    // else the non-terminal is processed in the state handlers
#ifdef TRACE_AST_STATE
    else
        TRACE("non-terminal: %s:%s", ptr->non_term->text, ptr->non_term->name);
#endif

    POST_STATE;
    RETURN;
}

/*
 * rule_element_list {
 *     OPAREN +rule_element CPAREN |
 *     +rule_element
 * }
 */
static void traverse_rule_element_list(struct _ast_rule_element_list_t_* ptr,
                                       ast_state_t* state) {

    ENTER;
    PRE_STATE;

    int post = 0;
    ast_rule_element_t* item = NULL;

    while(NULL != (item = iterate_pointer_list(ptr->rule_elems, &post)))
        traverse_rule_element(item, state);

    POST_STATE;
    RETURN;
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

    ENTER;
    PRE_STATE;

    switch(get_ast_node_type(ptr->func)) {
        case AST_OR_FUNC:
            traverse_or_func((ast_or_func_t*)ptr->func, state);
            break;
        case AST_ZERO_OR_MORE_FUNC:
            traverse_zero_or_more_func((ast_zero_or_more_func_t*)ptr->func, state);
            break;
        case AST_ZERO_OR_ONE_FUNC:
            traverse_zero_or_one_func((ast_zero_or_one_func_t*)ptr->func, state);
            break;
        case AST_ONE_OR_MORE_FUNC:
            traverse_one_or_more_func((ast_one_or_more_func_t*)ptr->func, state);
            break;
        default:
            // should be impossible
            fatal_error("unknown struct type in %s: %d", __func__, ptr->func->type);
    }

    POST_STATE;
    RETURN;
}

/*
 * one_or_more_func {
 *     ONE_OR_MORE rule_element_list
 * }
 */
static void traverse_one_or_more_func(struct _ast_one_or_more_func_t_* ptr,
                                      ast_state_t* state) {

    ENTER;
    PRE_STATE;

    traverse_rule_element_list(ptr->rel, state);

    POST_STATE;
    RETURN;
}

/*
 * zero_or_one_func {
 *     ZERO_OR_ONE rule_element_list
 * }
 */
static void traverse_zero_or_one_func(struct _ast_zero_or_one_func_t_* ptr,
                                      ast_state_t* state) {

    ENTER;
    PRE_STATE;

    traverse_rule_element_list(ptr->rel, state);

    POST_STATE;
    RETURN;
}

/*
 * zero_or_more_func {
 *     ZERO_OR_MORE rule_element_list
 * }
 */
static void traverse_zero_or_more_func(struct _ast_zero_or_more_func_t_* ptr,
                                       ast_state_t* state) {

    ENTER;
    PRE_STATE;

    traverse_rule_element_list(ptr->rel, state);

    POST_STATE;
    RETURN;
}

/*
 * or_func {
 *     PIPE rule_element_list
 * }
 */
static void traverse_or_func(struct _ast_or_func_t_* ptr, ast_state_t* state) {

    ENTER;
    PRE_STATE;

    traverse_rule_element_list(ptr->rel, state);

    POST_STATE;
    RETURN;
}

static size_t get_ast_node_size(ast_type_t type) {

    return (type == AST_GRAMMAR)            ? sizeof(ast_grammar_t) :
            (type == AST_RULE)              ? sizeof(ast_rule_t) :
            (type == AST_TERMINAL)          ? sizeof(ast_terminal_t) :
            (type == AST_RULE_ELEMENT)      ? sizeof(ast_rule_element_t) :
            (type == AST_RULE_ELEMENT_LIST) ? sizeof(ast_rule_element_list_t) :
            (type == AST_RULE_FUNCTION)     ? sizeof(ast_rule_function_t) :
            (type == AST_ONE_OR_MORE_FUNC)  ? sizeof(ast_one_or_more_func_t) :
            (type == AST_ZERO_OR_ONE_FUNC)  ? sizeof(ast_zero_or_one_func_t) :
            (type == AST_ZERO_OR_MORE_FUNC) ? sizeof(ast_zero_or_more_func_t) :
            (type == AST_OR_FUNC)           ? sizeof(ast_or_func_t) :
                                              (size_t)-1;
}

ast_node_t* create_ast_node(ast_type_t type) {

    ast_node_t* ptr = malloc(get_ast_node_size(type));
    assert(ptr != NULL);

    ptr->type = type;

    return ptr;
}

ast_type_t get_ast_node_type(void* node) {

    return ((ast_node_t*)node)->type;
}

void traverse_ast(void* grammar, void* state) {

    ENTER;
    traverse_grammar((ast_grammar_t*)grammar, (ast_state_t*)state);
    RETURN;
}
