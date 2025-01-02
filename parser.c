
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

static ast_grammar_t* parse_grammar(parser_state_t* pstate);
static ast_rule_t* parse_rule(parser_state_t* pstate);
static ast_rule_element_t* parse_rule_element(parser_state_t* pstate);
static ast_one_or_more_func_t* parse_one_or_more_func(parser_state_t* pstate);
static ast_zero_or_one_func_t* parse_zero_or_one_func(parser_state_t* pstate);
static ast_zero_or_more_func_t* parse_zero_or_more_func(parser_state_t* pstate);
static ast_or_func_t* parse_or_func(parser_state_t* pstate);
static ast_group_func_t* parse_group_func(parser_state_t* pstate);


/*
 * grammar {
 *     +rule END_OF_INPUT
 * }
 *
 */
static ast_grammar_t* parse_grammar(parser_state_t* pstate) {

    ENTER;

    assert(pstate != NULL);
    ast_grammar_t* ptr = NULL;

    int state = 0;
    bool finished = false;

    int post = post_token_queue();

    ast_rule_t* rule = NULL;
    pointer_list_t* list = NULL;

    while(!finished) {
        switch(state) {
            case 100:
                TRACE;
                if(NULL != (rule = parse_rule(pstate))) {
                    list = create_pointer_list();
                    add_pointer_list(list, rule);
                    state = 110;
                }
                else {
                    PARSE_ERROR("grammar must contain at least one rule");
                    state = 3000;
                }
                break;

            case 110:
                TRACE;
                if(NULL != (rule = parse_rule(pstate)))
                    add_pointer_list(list, rule);
                else
                    state = 120;
                break;

            case 120:
                TRACE;
                if(TTYPE == END_OF_INPUT) {
                    consume_token();
                    state = 1000;
                }
                else {
                    EXPECTED("end of input");
                    state = 3000;
                }
                break;

            case 1000:
                TRACE;
                ptr = create_ast_node(AST_);
                ptr->rules = list;
                finished = true;
                break;

            case 2000:
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case 3000:
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("FATAL: unknown state in %s: %d\n", __func__, state);
        }
    }

    RETURN(ptr);
}

/*
 * rule {
 *     NON_TERMINAL '{' +rule_element '}'
 * }
 *
 */
static ast_rule_t* parse_rule(parser_state_t* pstate) {

    ENTER;

    assert(pstate != NULL);
    ast_rule_t* ptr = NULL;

    int state = 0;
    bool finished = false;

    int post = post_token_queue();

    token_t* nterm;
    pointer_list_t* rule_elems;
    ast_rule_element_t* elem;

    while(!finished) {
        switch(state) {
            case 100:
                TRACE;
                if(TTYPE == NON_TERMINAL) {
                    nterm = get_token();
                    consume_token();
                    state = 110;
                }
                else {
                    EXPECTED("a non-terminal symbol");
                    state = 3000;
                }
                break;

            case 110:
                TRACE;
                if(TTYPE == OCBRACE) {
                    consume_token();
                    state = 120;
                }
                else {
                    EXPECTED("a \"{\"");
                    state = 3000;
                }
                break;

            case 120:
                TRACE;
                if(NULL != (elem = parse_rule_element(pstate))) {
                    rule_elems = create_pointer_list();
                    add_pointer_list(rule_elems, elem);
                    state = 130;
                }
                else {
                    PARSE_ERROR("at least one rule element is required in a rule");
                    state = 3000;
                }
                break;

            case 130:
                TRACE;
                if(NULL != (elem = parse_rule_element(pstate)))
                    add_pointer_list(rule_elems, elem);
                else
                    state = 140;
                break;

            case 140:
                TRACE;
                if(TTYPE == CCBRACE) {
                    consume_token();
                    state = 1000;
                }
                else {
                    EXPECTED("a \"}\"");
                    state = 3000;
                }
                break;

            case 1000:
                TRACE;
                ptr = create_ast_node(AST_);
                ptr->nterm = nterm;
                ptr->rule_elems = rule_elems;
                finished = true;
                break;

            case 2000:
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case 3000:
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("FATAL: unknown state in %s: %d\n", __func__, state);
        }
    }

    RETURN(ptr);
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
static ast_rule_element_t* parse_rule_element(parser_state_t* pstate) {

    ENTER;

    assert(pstate != NULL);
    ast_rule_element_t* ptr = NULL;

    int state = 0;
    bool finished = false;

    int post = post_token_queue();

    token_t* term = NULL;
    ast_node_t* nterm = NULL;

    while(!finished) {
        switch(state) {
            case 100:
                TRACE;
                if(TTYPE == NON_TERMINAL) {
                    term = get_token();
                    state = 1000;
                }
                else
                    state = 110;
                break;

            case 110:
                TRACE;
                if(TTYPE == TERMINAL_NAME) {
                    term = get_token();
                    state = 1000;
                }
                else
                    state = 120;
                break;

            case 120:
                TRACE;
                if(TTYPE == TERMINAL_OPER) {
                    term = get_token();
                    state = 1000;
                }
                else
                    state = 130;
                break;

            case 130:
                TRACE;
                if(TTYPE == TERMINAL_SYMBOL) {
                    term = get_token();
                    state = 1000;
                }
                else
                    state = 140;
                break;

            case 140:
                TRACE;
                if(NULL != (nterm = parse_or_func(pstate)))
                    state = 1000;
                else
                    state = 150;
                break;

            case 150:
                TRACE;
                if(NULL != (nterm = parse_zero_or_more_func(pstate)))
                    state = 1000;
                else
                    state = 160;
                break;

            case 160:
                TRACE;
                if(NULL != (nterm = parse_zero_or_one_func(pstate)))
                    state = 1000;
                else
                    state = 170;
                break;

            case 170:
                TRACE;
                if(NULL != (nterm = parse_one_or_more_func(pstate)))
                    state = 1000;
                else
                    state = 180;
                break;

            case 180:
                TRACE;
                if(NULL != (nterm = parse_group_func(pstate)))
                    state = 1000;
                else {
                    EXPECTED("a function or a terminal");
                    state = 3000;
                }
                break;

            case 1000:
                TRACE;
                ptr = create_ast_node(AST_);
                ptr->term = term;
                ptr->nterm = nterm;
                finished = true;
                break;

            case 2000:
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case 3000:
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("FATAL: unknown state in %s: %d\n", __func__, state);
        }
    }

    RETURN(ptr);
}

/*
 * one_or_more_func {
 *     '+' rule_element
 * }
 *
 */
static ast_one_or_more_func_t* parse_one_or_more_func(parser_state_t* pstate) {

    ENTER;

    assert(pstate != NULL);
    ast_one_or_more_func_t* ptr = NULL;

    int state = 0;
    bool finished = false;

    int post = post_token_queue();

    while(!finished) {
        switch(state) {
            case 100:
                TRACE;
                break;

            case 1000:
                TRACE;
                ptr = create_ast_node(AST_);
                finished = true;
                break;

            case 2000:
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case 3000:
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("FATAL: unknown state in %s: %d\n", __func__, state);
        }
    }

    RETURN(ptr);
}

/*
 * zero_or_one_func {
 *     '?' rule_element
 * }
 *
 */
static ast_zero_or_one_func_t* parse_zero_or_one_func(parser_state_t* pstate) {

    ENTER;

    assert(pstate != NULL);
    ast_zero_or_one_func_t* ptr = NULL;

    int state = 0;
    bool finished = false;

    int post = post_token_queue();

    while(!finished) {
        switch(state) {
            case 100:
                TRACE;
                break;

            case 1000:
                TRACE;
                ptr = create_ast_node(AST_);
                finished = true;
                break;

            case 2000:
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case 3000:
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("FATAL: unknown state in %s: %d\n", __func__, state);
        }
    }

    RETURN(ptr);
}

/*
 * zero_or_more_func {
 *     '*' rule_element
 * }
 *
 */
static ast_zero_or_more_func_t* parse_zero_or_more_func(parser_state_t* pstate) {

    ENTER;

    assert(pstate != NULL);
    ast_zero_or_more_func_t* ptr = NULL;

    int state = 0;
    bool finished = false;

    int post = post_token_queue();

    while(!finished) {
        switch(state) {
            case 100:
                TRACE;
                break;

            case 1000:
                TRACE;
                ptr = create_ast_node(AST_);
                finished = true;
                break;

            case 2000:
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case 3000:
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("FATAL: unknown state in %s: %d\n", __func__, state);
        }
    }

    RETURN(ptr);
}

/*
 * or_func {
 *     '|' rule_element
 * }
 *
 */
static ast_or_func_t* parse_or_func(parser_state_t* pstate) {

    ENTER;

    assert(pstate != NULL);
    ast_or_func_t* ptr = NULL;

    int state = 0;
    bool finished = false;

    int post = post_token_queue();

    while(!finished) {
        switch(state) {
            case 100:
                TRACE;
                break;

            case 1000:
                TRACE;
                ptr = create_ast_node(AST_);
                finished = true;
                break;

            case 2000:
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case 3000:
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("FATAL: unknown state in %s: %d\n", __func__, state);
        }
    }

    RETURN(ptr);
}

/*
 * group_func {
 *     '(' +rule_element ')'
 * }
 *
 */
static ast_group_func_t* parse_group_func(parser_state_t* pstate) {

    ENTER;

    assert(pstate != NULL);
    ast_group_func_t* ptr = NULL;

    RETURN(ptr);
}

/*
 * Set up the parser to run.
 */
static parser_state_t* init_parser(const char* file_name) {

    ENTER;

    init_scanner(file_name);
    parser_state_t* ptr = _ALLOC_DS(parser_state_t);
    ptr->state = 0;

    RETURN(ptr);
}

/*
 * Public interface to the parser.
 */
void* parse(const char* file_name) {

    ENTER;

    assert(file_name != NULL);
    void* ptr = parse_grammar(init_parser(file_name));

    RETURN(ptr);
}

