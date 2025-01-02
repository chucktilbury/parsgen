#ifndef _AST_H_
#define _AST_H_

#include "pointer_list.h"
#include "scanner.h"

typedef enum {
    AST_GRAMMAR,
    AST_RULE,
    AST_TERMINAL,
    AST_RULE_ELEMENT,
    AST_RULE_ELEMENT_LIST,
    AST_RULE_FUNCTION,
    AST_ONE_OR_MORE_FUNC,
    AST_ZERO_OR_ONE_FUNC,
    AST_ZERO_OR_MORE_FUNC,
    AST_OR_FUNC,
} ast_type_t;

/*
 * Support data structure casts.
 */
typedef int (*ast_callback_t)(void*, void*);

typedef struct _ast_state_t_ {
    ast_callback_t pre;
    ast_callback_t post;
    void* state;
} ast_state_t;

typedef struct _ast_node_t_ {
    ast_type_t type;
} ast_node_t;

/*
 * grammar {
 *    +rule
 * }
 */
typedef struct _ast_grammar_t_ {
    ast_node_t node;
    pointer_list_t* rules;
} ast_grammar_t;

/*
 * rule {
 *    NON_TERMINAL OCURLY rule_element_list CCURLY
 * }
 */
typedef struct _ast_rule_t_ {
    ast_node_t node;
    token_t* non_term;
    struct _ast_rule_element_list_t_* elem_lst;
} ast_rule_t;

/*
 * terminal {
 *     TERMINAL_NAME |
 *     TERMINAL_OPER |
 *     TERMINAL_SYMBOL
 * }
 */
typedef struct _ast_terminal_t_ {
    ast_node_t node;
    token_t* term;
} ast_terminal_t;

/*
 * rule_element {
 *     NON_TERMINAL |
 *     terminal |
 *     rule_function
 * }
 */
typedef struct _ast_rule_element_t_ {
    ast_node_t node;
    token_t* non_term;
    struct _ast_terminal_t_* term;
    struct _ast_rule_function_t_* function;
} ast_rule_element_t;

/*
 * rule_element_list {
 *     OPAREN +rule_element CPAREN |
 *     +rule_element
 * }
 */
typedef struct _ast_rule_element_list_t_ {
    ast_node_t node;
    pointer_list_t* rule_elems;
} ast_rule_element_list_t;

/*
 * rule_function {
 *     or_func |
 *     zero_or_more_func |
 *     zero_or_one_func |
 *     one_or_more_func
 * }
 */
typedef struct _ast_rule_function_t_ {
    ast_node_t node;
    ast_node_t* func;
} ast_rule_function_t;

/*
 * one_or_more_func {
 *     ONE_OR_MORE rule_element_list
 * }
 */
typedef struct _ast_one_or_more_func_t_ {
    ast_node_t node;
    struct _ast_rule_element_list_t_* rel;
} ast_one_or_more_func_t;

/*
 * zero_or_one_func {
 *     ZERO_OR_ONE rule_element_list
 * }
 */
typedef struct _ast_zero_or_one_func_t_ {
    ast_node_t node;
    struct _ast_rule_element_list_t_* rel;
} ast_zero_or_one_func_t;

/*
 * zero_or_more_func {
 *     ZERO_OR_MORE rule_element_list
 * }
 */
typedef struct _ast_zero_or_more_func_t_ {
    ast_node_t node;
    struct _ast_rule_element_list_t_* rel;
} ast_zero_or_more_func_t;

/*
 * or_func {
 *     rule_element_list PIPE rule_element_list
 * }
 */
typedef struct _ast_or_func_t_ {
    ast_node_t node;
    struct _ast_rule_element_list_t_* rel;
} ast_or_func_t;

ast_node_t* create_ast_node(ast_type_t type);
ast_type_t get_ast_node_type(void* node);
void traverse_ast(void*, void*);

#endif /* _AST_H_ */
