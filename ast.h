#ifndef _AST_H_
#define _AST_H_

#include "pointer_list.h"
#include "scanner.h"

typedef enum {
    AST_GRAMMAR,
    AST_RULE,
    AST_RULE_ELEMENT,
    AST_ONE_OR_MORE_FUNC,
    AST_ZERO_OR_ONE_FUNC,
    AST_ZERO_OR_MORE_FUNC,
    AST_OR_FUNC,
    AST_GROUP_FUNC,
} ast_type_t;

typedef struct _ast_node_t_ {
    ast_type_t type;
} ast_node_t;

typedef int (*ast_callback_t)(void*, void*);

typedef struct _ast_state_t_ {
    ast_callback_t pre;
    ast_callback_t post;
    void* state;
} ast_state_t;

/*
 * grammar {
 *     +rule END_OF_INPUT
 * }
 *
 */
typedef struct _ast_grammar_t_ {
    ast_node_t node;
    pointer_list_t* rules;
} ast_grammar_t;

/*
 * rule {
 *     NON_TERMINAL '{' +rule_element '}'
 * }
 *
 */
typedef struct _ast_rule_t_ {
    ast_node_t node;
    token_t* nterm;
    pointer_list_t* rule_elems;
} ast_rule_t;

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
typedef struct _ast_rule_element_t_ {
    ast_node_t node;
    // if term is not NULL, check the token type. If the nterm is set then
    // check the node type.
    token_t* term;
    ast_node_t* nterm;
} ast_rule_element_t;

/*
 * one_or_more_func {
 *     '+' rule_element
 * }
 *
 */
typedef struct _ast_one_or_more_func_t_ {
    ast_node_t node;
    struct _ast_rule_element_t_* elem;
} ast_one_or_more_func_t;

/*
 * zero_or_one_func {
 *     '?' rule_element
 * }
 *
 */
typedef struct _ast_zero_or_one_func_t_ {
    ast_node_t node;
    struct _ast_rule_element_t_* elem;
} ast_zero_or_one_func_t;

/*
 * zero_or_more_func {
 *     '*' rule_element
 * }
 *
 */
typedef struct _ast_zero_or_more_func_t_ {
    ast_node_t node;
    struct _ast_rule_element_t_* elem;
} ast_zero_or_more_func_t;

/*
 * or_func {
 *     '|' rule_element
 * }
 *
 */
typedef struct _ast_or_func_t_ {
    ast_node_t node;
    struct _ast_rule_element_t_* elem;
} ast_or_func_t;

/*
 * group_func {
 *     '(' +rule_element ')'
 * }
 *
 */
typedef struct _ast_group_func_t {
    ast_node_t node;
    pointer_list_t* list;
} ast_group_func_t;

void traverse_ast(void* ptr, void* state);
ast_node_t* create_ast_node(ast_type_t type);
ast_type_t get_ast_node_type(void* node);

#endif /* _AST_H_ */
