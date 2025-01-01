
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "errors.h"
#include "memory.h"
#include "parser.h"
#include "pointer_list.h"
#include "scanner.h"

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

// #define TRACE_PARSER_STATE

#ifdef TRACE_PARSER_STATE
static int depth = 0;
#define DINC 2
#define TRACE                                                      \
    do {                                                           \
        token_t* tok = get_token();                                \
        fprintf(stdout, "%*sSTATE: %d: %s:%d\n", depth, "", state, \
                tok->name ? tok->name : tok->text, tok->line_no);  \
    } while(false)
#define ENTER                                                   \
    do {                                                        \
        fprintf(stdout, "%*sENTER: %s\n", depth, "", __func__); \
        depth += DINC;                                          \
    } while(false)
#define RETURN(v)                                                              \
    do {                                                                       \
        depth -= DINC;                                                         \
        fprintf(stdout, "%*sRETURN(%p): %s\n", depth, "", (void*)v, __func__); \
        return (v);                                                            \
    } while(false)
#else
#define TRACE
#define ENTER
#define RETURN(v) return (v)
#endif

#define TTYPE (get_token()->type)

#define PARSE_ERROR(f, ...)                                                    \
    do {                                                                       \
        syntax_error(get_file_name(), get_token()->line_no, f, ##__VA_ARGS__); \
    } while(false)

#define EXPECTED(what)                                                      \
    do {                                                                    \
        PARSE_ERROR("expected %s but got \"%s\"", what, get_token()->text); \
    } while(false)

/*
 * grammar {
 *    +rule END_OF_INPUT
 * }
 */
static ast_grammar_t* parse_grammar(parser_state_t* pstate) {

    assert(pstate != NULL);
    ENTER;

    ast_grammar_t* retv = NULL;

    int post = post_token_queue();
    int state = 0;
    bool finished = false;

    ast_rule_t* rule = NULL;
    pointer_list_t* list = NULL;

    while(!finished) {
        switch(state) {
            case 0:
                // initial state, one or more rule.
                TRACE;
                if(NULL != (rule = parse_rule(pstate))) {
                    list = create_pointer_list();
                    add_pointer_list(list, rule);
                    state = 1;
                }
                else {
                    // error
                    EXPECTED("a rule definition");
                    state = 102;
                }
                break;

            case 1:
                // get rules until EOI
                TRACE;
                if(TTYPE == END_OF_INPUT) {
                    state = 100;
                    consume_token();
                }
                else if(NULL != (rule = parse_rule(pstate))) {
                    add_pointer_list(list, rule);
                    // state stays the same
                }
                else {
                    // error
                    EXPECTED("a rule definition or the end of input");
                    state = 102;
                }
                break;

            case 100:
                // match made
                TRACE;
                retv = (ast_grammar_t*)create_ast_node(AST_GRAMMAR);
                retv->rules = list;
                finished = true;
                break;

            case 101:
                // no match
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case 102:
                // error return
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("FATAL: unknown state in %s: %d\n", __func__, state);
                exit(1);
        }
    }

    RETURN(retv);
}

/*
 * rule {
 *    NON_TERMINAL OCURLY rule_element_list CCURLY
 * }
 */
static ast_rule_t* parse_rule(parser_state_t* pstate) {

    assert(pstate != NULL);
    ENTER;

    ast_rule_t* retv = NULL;

    int post = post_token_queue();
    int state = 0;
    bool finished = false;

    token_t* non_term = NULL;
    ast_rule_element_list_t* rel = NULL;

    while(!finished) {
        switch(state) {
            case 0:
                // initial state
                // get the static sequence
                TRACE;
                if(TTYPE == NON_TERMINAL) {
                    non_term = get_token();
                    consume_token();
                    state = 1;
                }
                else {
                    state = 101;
                }
                break;

            case 1:
                TRACE;
                if(TTYPE == OCURLY) {
                    consume_token();
                    state = 2;
                }
                else {
                    EXPECTED("a \"{\"");
                    state = 102;
                }
                break;

            case 2:
                TRACE;
                if(NULL != (rel = parse_rule_element_list(pstate)))
                    state = 3;
                else {
                    EXPECTED("a rule body");
                    state = 102;
                }
                break;

            case 3:
                TRACE;
                if(TTYPE == CCURLY) {
                    consume_token();
                    state = 100;
                }
                else {
                    EXPECTED("a \"}\"");
                    state = 102;
                }
                break;


            case 100:
                // match made
                TRACE;
                retv = (ast_rule_t*)create_ast_node(AST_RULE);
                retv->elem_lst = rel;
                retv->non_term = non_term;
                finished = true;
                break;

            case 101:
                // no match
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case 102:
                // error return
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("FATAL: unknown state in %s: %d\n", __func__, state);
                exit(1);
        }
    }

    RETURN(retv);
}

/*
 * terminal {
 *     TERMINAL_NAME |
 *     TERMINAL_OPER |
 *     TERMINAL_SYMBOL
 * }
 */
static ast_terminal_t* parse_terminal(parser_state_t* pstate) {

    assert(pstate != NULL);
    ENTER;

    ast_terminal_t* retv = NULL;
    int post = post_token_queue();
    int state = 0;
    bool finished = false;

    while(!finished) {
        switch(state) {
            case 00:
                // initial state
                TRACE;
                if(TTYPE == TERMINAL_NAME)
                    state = 100;
                else
                    state = 10;
                break;

            case 10:
                TRACE;
                if(TTYPE == TERMINAL_OPER)
                    state = 100;
                else
                    state = 20;
                break;

            case 20:
                TRACE;
                if(TTYPE == TERMINAL_SYMBOL)
                    state = 100;
                else
                    state = 101;
                break;

            case 100:
                // match made
                TRACE;
                retv = (ast_terminal_t*)create_ast_node(AST_TERMINAL);
                retv->term = get_token();
                consume_token();
                finished = true;
                break;

            case 101:
                // no match
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case 102:
                // error return
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("FATAL: unknown state in %s: %d\n", __func__, state);
                exit(1);
        }
    }

    RETURN(retv);
}

/*
 * rule_element {
 *     NON_TERMINAL |
 *     terminal |
 *     rule_function
 * }
 */
static ast_rule_element_t* parse_rule_element(parser_state_t* pstate) {

    assert(pstate != NULL);
    ENTER;

    ast_rule_element_t* retv = NULL;
    int post = post_token_queue();
    int state = 0;
    bool finished = false;

    token_t* non_term = NULL;
    ast_terminal_t* term = NULL;
    ast_rule_function_t* function = NULL;

    while(!finished) {
        switch(state) {
            case 00:
                // initial state
                TRACE;
                if(TTYPE == NON_TERMINAL) {
                    non_term = get_token();
                    consume_token();
                    state = 100;
                }
                else
                    state = 10;
                break;

            case 10:
                // initial state
                TRACE;
                if(NULL != (term = parse_terminal(pstate)))
                    state = 100;
                else
                    state = 20;
                break;

            case 20:
                // initial state
                TRACE;
                if(NULL != (function = parse_rule_function(pstate)))
                    state = 100;
                else {
                    // EXPECTED("a terminal, non-terminal, or a rule function");
                    // state = 102;
                    state = 101;
                }
                break;

            case 100:
                // match made
                TRACE;
                retv = (ast_rule_element_t*)create_ast_node(AST_RULE_ELEMENT);
                retv->non_term = non_term;
                retv->term = term;
                retv->function = function;
                finished = true;
                break;

            case 101:
                // no match
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case 102:
                // error return
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("FATAL: unknown state in %s: %d\n", __func__, state);
                exit(1);
        }
    }

    RETURN(retv);
}

/*
 * rule_element_list {
 *     OPAREN +rule_element CPAREN |
 *     +rule_element
 * }
 */
static ast_rule_element_list_t* parse_rule_element_list(parser_state_t* pstate) {

    assert(pstate != NULL);
    ENTER;

    ast_rule_element_list_t* retv = NULL;
    int post = post_token_queue();
    int state = 0;
    bool finished = false;

    ast_rule_element_t* re = NULL;
    pointer_list_t* list = create_pointer_list();

    while(!finished) {
        switch(state) {
            case 00:
                // initial state
                TRACE;
                if(TTYPE == OPAREN) {
                    consume_token();
                    state = 10; // a CPAREN is required at the end
                }
                else
                    state = 20; // no CPAREN required
                break;

            case 10:
                TRACE;
                if(NULL != (re = parse_rule_element(pstate))) {
                    add_pointer_list(list, re);
                    state = 11;
                }
                else {
                    EXPECTED("a terminal, non-terminal, or a rule function");
                    state = 102;
                }
                break;


            case 11:
                TRACE;
                if(NULL != (re = parse_rule_element(pstate)))
                    add_pointer_list(list, re);
                else
                    state = 12;
                break;


            case 12:
                TRACE;
                if(TTYPE == CPAREN) {
                    consume_token();
                    state = 100;
                }
                else {
                    EXPECTED("a \")\"");
                    state = 102;
                }
                break;

            case 20:
                TRACE;
                if(NULL != (re = parse_rule_element(pstate))) {
                    add_pointer_list(list, re);
                    state = 21;
                }
                else
                    state = 101;
                break;


            case 21:
                TRACE;
                if(NULL != (re = parse_rule_element(pstate)))
                    add_pointer_list(list, re);
                else
                    state = 100;
                break;


            case 100:
                // match made
                TRACE;
                retv = (ast_rule_element_list_t*)create_ast_node(AST_RULE_ELEMENT_LIST);
                retv->rule_elems = list;
                finished = true;
                break;

            case 101:
                // no match
                TRACE;
                reset_token_queue(post);
                destroy_pointer_list(list);
                finished = true;
                break;

            case 102:
                // error return
                TRACE;
                destroy_pointer_list(list);
                finished = true;
                break;

            default:
                fatal_error("FATAL: unknown state in %s: %d\n", __func__, state);
                exit(1);
        }
    }

    RETURN(retv);
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

    assert(pstate != NULL);
    ENTER;

    ast_rule_function_t* retv = NULL;
    int post = post_token_queue();
    int state = 0;
    bool finished = false;

    ast_node_t* func = NULL;

    while(!finished) {
        switch(state) {
            case 00:
                // first option initial state
                TRACE;
                if(NULL != (func = (ast_node_t*)parse_or_func(pstate)))
                    state = 100;
                else
                    state = 10;
                break;

            case 10:
                // initial state, option 2
                TRACE;
                if(NULL != (func = (ast_node_t*)parse_zero_or_more_func(pstate)))
                    state = 100;
                else
                    state = 20;
                break;

            case 20:
                // initial state, option 3
                TRACE;
                if(NULL != (func = (ast_node_t*)parse_zero_or_one_func(pstate)))
                    state = 100;
                else
                    state = 30;
                break;

            case 30:
                // initial state, option 4
                TRACE;
                if(NULL != (func = (ast_node_t*)parse_one_or_more_func(pstate)))
                    state = 100;
                else
                    state = 101;
                break;

            case 100:
                // match made
                TRACE;
                retv = (ast_rule_function_t*)create_ast_node(AST_RULE_FUNCTION);
                retv->func = func;
                finished = true;
                break;

            case 101:
                // no match
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case 102:
                // error return
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("FATAL: unknown state in %s: %d\n", __func__, state);
                exit(1);
        }
    }

    RETURN(retv);
}

/*
 * one_or_more_func {
 *     ONE_OR_MORE rule_element_list
 * }
 */
static ast_one_or_more_func_t* parse_one_or_more_func(parser_state_t* pstate) {

    assert(pstate != NULL);
    ENTER;

    ast_one_or_more_func_t* retv = NULL;
    int post = post_token_queue();
    int state = 0;
    bool finished = false;

    ast_rule_element_list_t* rel = NULL;

    while(!finished) {
        switch(state) {
            case 00:
                // initial state
                TRACE;
                if(TTYPE == ONE_OR_MORE) {
                    consume_token();
                    state = 01;
                }
                else
                    state = 101;
                break;

            case 01:
                // initial state
                TRACE;
                if(NULL != (rel = parse_rule_element_list(pstate))) {
                    state = 100;
                }
                else {
                    EXPECTED("rule elements");
                    state = 102;
                }
                break;

            case 100:
                // match made
                TRACE;
                retv = (ast_one_or_more_func_t*)create_ast_node(AST_ONE_OR_MORE_FUNC);
                retv->rel = rel;
                finished = true;
                break;

            case 101:
                // no match
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case 102:
                // error return
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("FATAL: unknown state in %s: %d\n", __func__, state);
                exit(1);
        }
    }

    RETURN(retv);
}

/*
 * zero_or_one_func {
 *     ZERO_OR_ONE rule_element_list
 * }
 */
static ast_zero_or_one_func_t* parse_zero_or_one_func(parser_state_t* pstate) {

    assert(pstate != NULL);
    ENTER;

    ast_zero_or_one_func_t* retv = NULL;
    int post = post_token_queue();
    int state = 0;
    bool finished = false;

    ast_rule_element_list_t* rel = NULL;

    while(!finished) {
        switch(state) {
            case 00:
                // initial state
                TRACE;
                if(TTYPE == ZERO_OR_ONE) {
                    consume_token();
                    state = 01;
                }
                else
                    state = 101;
                break;

            case 01:
                // initial state
                TRACE;
                if(NULL != (rel = parse_rule_element_list(pstate))) {
                    state = 100;
                }
                else {
                    EXPECTED("rule elements");
                    state = 102;
                }
                break;

            case 100:
                // match made
                TRACE;
                retv = (ast_zero_or_one_func_t*)create_ast_node(AST_ZERO_OR_ONE_FUNC);
                retv->rel = rel;
                finished = true;
                break;

            case 101:
                // no match
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case 102:
                // error return
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("FATAL: unknown state in %s: %d\n", __func__, state);
                exit(1);
        }
    }

    RETURN(retv);
}

/*
 * zero_or_more_func {
 *     ZERO_OR_MORE rule_element_list
 * }
 */
static ast_zero_or_more_func_t* parse_zero_or_more_func(parser_state_t* pstate) {

    assert(pstate != NULL);
    ENTER;

    ast_zero_or_more_func_t* retv = NULL;
    int post = post_token_queue();
    int state = 0;
    bool finished = false;

    ast_rule_element_list_t* rel = NULL;

    while(!finished) {
        switch(state) {
            case 00:
                // initial state
                TRACE;
                if(TTYPE == ZERO_OR_MORE) {
                    consume_token();
                    state = 01;
                }
                else
                    state = 101;
                break;

            case 01:
                // initial state
                TRACE;
                if(NULL != (rel = parse_rule_element_list(pstate))) {
                    state = 100;
                }
                else {
                    EXPECTED("rule elements");
                    state = 102;
                }
                break;

            case 100:
                // match made
                TRACE;
                retv = (ast_zero_or_more_func_t*)create_ast_node(AST_ZERO_OR_MORE_FUNC);
                retv->rel = rel;
                finished = true;
                break;

            case 101:
                // no match
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case 102:
                // error return
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("FATAL: unknown state in %s: %d\n", __func__, state);
                exit(1);
        }
    }

    RETURN(retv);
}

/*
 * or_func {
 *     PIPE rule_element_list
 * }
 */
static ast_or_func_t* parse_or_func(parser_state_t* pstate) {

    assert(pstate != NULL);
    ENTER;

    ast_or_func_t* retv = NULL;
    int post = post_token_queue();
    int state = 0;
    bool finished = false;

    ast_rule_element_list_t* rel = NULL;

    while(!finished) {
        switch(state) {
            case 00:
                // initial state
                TRACE;
                if(TTYPE == PIPE) {
                    consume_token();
                    state = 01;
                }
                else
                    state = 101;
                break;

            case 01:
                // initial state
                TRACE;
                if(NULL != (rel = parse_rule_element_list(pstate))) {
                    state = 100;
                }
                else {
                    EXPECTED("rule elements");
                    state = 102;
                }
                break;

            case 100:
                // match made
                TRACE;
                retv = (ast_or_func_t*)create_ast_node(AST_OR_FUNC);
                retv->rel = rel;
                finished = true;
                break;

            case 101:
                // no match
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case 102:
                // error return
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("FATAL: unknown state in %s: %d\n", __func__, state);
                exit(1);
        }
    }

    RETURN(retv);
}

/*
 * Main interface into the parser.
 */

parser_state_t* init_parser(const char* file_name) {

    assert(file_name != NULL);
    ENTER;

    init_scanner(file_name);
    parser_state_t* ptr = _ALLOC_DS(parser_state_t); // malloc(sizeof(parser_state_t));
    ptr->state = 0;

    RETURN(ptr);
}

ast_grammar_t* parse(parser_state_t* ptr) {

    ENTER;
    ast_grammar_t* retv = parse_grammar(ptr);
    RETURN(retv);
}
