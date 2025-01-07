#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "errors.h"
#include "memory.h"
#include "pointer_list.h"
// #include "scanner.h"

#define PRE_STATE                                                  \
    do {                                                           \
        if(state != NULL) {                                        \
            if(((ast_state_t*)(state))->pre != NULL)               \
                (*((ast_state_t*)(state))->pre)((ast_node_t*)ptr); \
        }                                                          \
    } while(0)

#define POST_STATE                                                  \
    do {                                                            \
        if(state != NULL) {                                         \
            if(((ast_state_t*)(state))->post != NULL)               \
                (*((ast_state_t*)(state))->post)((ast_node_t*)ptr); \
        }                                                           \
    } while(0)

// #define TRACE_AST_STATE

#ifdef TRACE_AST_STATE
static int depth = 0;
static int num_states = 0;
#define DINC 2
#define TRACE(...)                                \
    do {                                          \
        fprintf(stdout, "%*sTRACE: ", depth, ""); \
        fprintf(stdout, __VA_ARGS__);             \
        fprintf(stdout, "\n");                    \
    } while(false)

#define ENTER                                                   \
    do {                                                        \
        fprintf(stdout, "%*sENTER: %s\n", depth, "", __func__); \
        depth += DINC;                                          \
        num_states++;                                           \
        PRE_STATE;                                              \
    } while(false)

#define RETURN                                                   \
    do {                                                         \
        depth -= DINC;                                           \
        fprintf(stdout, "%*sRETURN: %s\n", depth, "", __func__); \
        POST_STATE;                                              \
        return;                                                  \
    } while(false)

#define START                                                     \
    do {                                                          \
        depth = 0;                                                \
        num_states = 0;                                           \
        assert(ptr != NULL);                                      \
        fprintf(stdout, "\n%*sSTART: %s\n", depth, "", __func__); \
        depth += DINC;                                            \
    } while(false)

#define FINISH                                                              \
    do {                                                                    \
        depth -= DINC;                                                      \
        fprintf(stdout, "%*sFINISH: number of states: %d: %s\n", depth, "", \
                num_states, __func__);                                      \
        return;                                                             \
    } while(false)
#else
#define TRACE(...)
#define ENTER      \
    do {           \
        PRE_STATE; \
    } while(false)

#define RETURN      \
    do {            \
        POST_STATE; \
        return;     \
    } while(false)

#define START

#define FINISH RETURN

#endif

static void traverse_grammar(ast_grammar_t* ptr, ast_state_t* state);
static void traverse_non_terminal_rule(ast_non_terminal_rule_t* ptr, ast_state_t* state);
static void traverse_terminal_rule(ast_terminal_rule_t* ptr, ast_state_t* state);
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

    ENTER;

    ast_node_t* rule;
    int post = 0;

    while(NULL != (rule = iterate_pointer_list(ptr->rules, &post))) {
        if(get_ast_node_type(rule) == AST_NON_TERMINAL_RULE)
            traverse_non_terminal_rule((ast_non_terminal_rule_t*)rule, state);
        else
            traverse_terminal_rule((ast_terminal_rule_t*)rule, state);
    }

    RETURN;
}

/*
 * terminal_rule {
 *     TERMINAL_SYMBOL TERMINAL_EXPR
 * }
 *
 */
static void traverse_terminal_rule(ast_terminal_rule_t* ptr, ast_state_t* state) {

    ENTER;

    TRACE("terminal_symbol: %s:%s:%s:%d", ptr->term_sym->name, ptr->term_sym->text,
          tok_type_to_str(ptr->term_sym), ptr->term_sym->line_no);

    TRACE("terminal_expr: %s:%s:%s:%d", ptr->term_expr->name, ptr->term_expr->text,
          tok_type_to_str(ptr->term_expr), ptr->term_expr->line_no);

    RETURN;
}

/*
 * non_terminal_rule {
 *     NON_TERMINAL '{' +rule_element '}'
 * }
 *
 */
static void traverse_non_terminal_rule(ast_non_terminal_rule_t* ptr, ast_state_t* state) {

    ENTER;

    TRACE("non-terminal: %s:%s:%s:%d", ptr->nterm->name, ptr->nterm->text,
          tok_type_to_str(ptr->nterm), ptr->nterm->line_no);

    int post = 0;
    ast_rule_element_t* elem;

    while(NULL != (elem = iterate_pointer_list(ptr->rule_elems, &post)))
        traverse_rule_element(elem, state);

    RETURN;
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

    ENTER;

    if(ptr->nterm != NULL) {
        if(ptr->nterm->type == AST_OR_FUNC)
            traverse_or_func((ast_or_func_t*)ptr->nterm, state);
        else if(ptr->nterm->type == AST_ZERO_OR_MORE_FUNC)
            traverse_zero_or_more_func((ast_zero_or_more_func_t*)ptr->nterm, state);
        else if(ptr->nterm->type == AST_ONE_OR_MORE_FUNC)
            traverse_one_or_more_func((ast_one_or_more_func_t*)ptr->nterm, state);
        else if(ptr->nterm->type == AST_ZERO_OR_ONE_FUNC)
            traverse_zero_or_one_func((ast_zero_or_one_func_t*)ptr->nterm, state);
        else if(ptr->nterm->type == AST_GROUP_FUNC)
            traverse_group_func((ast_group_func_t*)ptr->nterm, state);
        else
            fatal_error("unknown non-terminal symbol in %s", __func__);
    }
#ifdef TRACE_AST_STATE
    else if(ptr->term != NULL) {
        if(ptr->term->type == NON_TERMINAL)
            TRACE("non-terminal: %s:%s:%s:%d", ptr->term->name, ptr->term->text, tok_type_to_str(ptr->term), ptr->term->line_no);
        else if(ptr->term->type == TERMINAL_NAME)
            TRACE("terminal name: %s:%s:%s:%d", ptr->term->name, ptr->term->text, tok_type_to_str(ptr->term), ptr->term->line_no);
        else if(ptr->term->type == TERMINAL_OPER)
            TRACE("terminal oper: %s:%s:%s:%d", ptr->term->name, ptr->term->text, tok_type_to_str(ptr->term), ptr->term->line_no);
        else if(ptr->term->type == TERMINAL_SYMBOL)
            TRACE("terminal symbol: %s:%s:%s:%d", ptr->term->name, ptr->term->text, tok_type_to_str(ptr->term), ptr->term->line_no);
        else
            fatal_error("unknown terminal symbol in %s", __func__);
    }
    else
        fatal_error("unknown state in %s", __func__);
#endif

    RETURN;
}

/*
 * one_or_more_func {
 *     '+' rule_element
 * }
 *
 */
static void traverse_one_or_more_func(ast_one_or_more_func_t* ptr, ast_state_t* state) {

    ENTER;

    traverse_rule_element(ptr->elem, state);

    RETURN;
}

/*
 * zero_or_one_func {
 *     '?' rule_element
 * }
 *
 */
static void traverse_zero_or_one_func(ast_zero_or_one_func_t* ptr, ast_state_t* state) {

    ENTER;

    traverse_rule_element(ptr->elem, state);

    RETURN;
}

/*
 * zero_or_more_func {
 *     '*' rule_element
 * }
 *
 */
static void traverse_zero_or_more_func(ast_zero_or_more_func_t* ptr, ast_state_t* state) {

    ENTER;

    traverse_rule_element(ptr->elem, state);

    RETURN;
}

/*
 * or_func {
 *     '|' rule_element
 * }
 *
 */
static void traverse_or_func(ast_or_func_t* ptr, ast_state_t* state) {

    ENTER;

    traverse_rule_element(ptr->elem, state);

    RETURN;
}

/*
 * group_func {
 *     '(' +rule_element ')'
 * }
 *
 */
static void traverse_group_func(ast_group_func_t* ptr, ast_state_t* state) {

    ENTER;

    int post = 0;
    ast_rule_element_t* elem;

    while(NULL != (elem = iterate_pointer_list(ptr->list, &post)))
        traverse_rule_element(elem, state);

    RETURN;
}

static size_t get_ast_node_size(ast_type_t type) {

    return (type == AST_GRAMMAR)            ? sizeof(ast_grammar_t) :
            (type == AST_NON_TERMINAL_RULE) ? sizeof(ast_non_terminal_rule_t) :
            (type == AST_TERMINAL_RULE)     ? sizeof(ast_terminal_rule_t) :
            (type == AST_RULE_ELEMENT)      ? sizeof(ast_rule_element_t) :
            (type == AST_ONE_OR_MORE_FUNC)  ? sizeof(ast_one_or_more_func_t) :
            (type == AST_ZERO_OR_ONE_FUNC)  ? sizeof(ast_zero_or_one_func_t) :
            (type == AST_ZERO_OR_MORE_FUNC) ? sizeof(ast_zero_or_more_func_t) :
            (type == AST_OR_FUNC)           ? sizeof(ast_or_func_t) :
            (type == AST_GROUP_FUNC)        ? sizeof(ast_group_func_t) :
                                              (size_t)-1;
}

void traverse_ast(void* ptr, void* state) {

    START;

    traverse_grammar((ast_grammar_t*)ptr, (ast_state_t*)state);

    FINISH;
}

ast_type_t get_ast_node_type(void* node) {

    return ((ast_node_t*)node)->type;
}

ast_node_t* create_ast_node(ast_type_t type) {

    ast_node_t* ptr = _ALLOC(get_ast_node_size(type));
    ptr->type = type;

    return ptr;
}
