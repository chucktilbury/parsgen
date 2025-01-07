
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

#define MATCH_STATE 1000
#define NO_MATCH_STATE 2000
#define ERROR_STATE 3000

// #define TRACE_PARSER_STATE

#ifdef TRACE_PARSER_STATE
static int depth = 0;
static int num_states = 0;
#define DINC 2
#define TRACE                                                              \
    do {                                                                   \
        token_t* tok = get_token();                                        \
        fprintf(stdout, "%*sSTATE: %d: %s:%s:%s:%d\n", depth, "", state,   \
                tok_type_to_str(tok), tok->name, tok->text, tok->line_no); \
        num_states++;                                                      \
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
#define START           \
    do {                \
        depth = 0;      \
        num_states = 0; \
        ENTER;          \
    } while(false)
#define FINISH(v)                                                        \
    do {                                                                 \
        fprintf(stdout, "%*stotal states: %d\n", depth, "", num_states); \
        RETURN(v);                                                       \
    } while(false)
#else
#define TRACE
#define ENTER
#define RETURN(v) return (v)
#define START
#define FINISH(v) RETURN(v)
#endif

#define TTYPE (get_token()->type)

#define PARSE_ERROR(...)                                                  \
    do {                                                                  \
        syntax_error(get_file_name(), get_token()->line_no, __VA_ARGS__); \
    } while(false)

#define EXPECTED(what)                                                      \
    do {                                                                    \
        PARSE_ERROR("expected %s but got \"%s\"", what, get_token()->text); \
    } while(false)

static ast_grammar_t* parse_grammar(parser_state_t* pstate);
static ast_non_terminal_rule_t* parse_non_terminal_rule(parser_state_t* pstate);
static ast_terminal_rule_t* parse_terminal_rule(parser_state_t* pstate);
static ast_rule_element_t* parse_rule_element(parser_state_t* pstate);
static ast_one_or_more_func_t* parse_one_or_more_func(parser_state_t* pstate);
static ast_zero_or_one_func_t* parse_zero_or_one_func(parser_state_t* pstate);
static ast_zero_or_more_func_t* parse_zero_or_more_func(parser_state_t* pstate);
static ast_or_func_t* parse_or_func(parser_state_t* pstate);
static ast_group_func_t* parse_group_func(parser_state_t* pstate);


/*
 * grammar {
 *    +(non_terminal_rule | terminal_rule) END_OF_INPUT
 * }
 *
 */
static ast_grammar_t* parse_grammar(parser_state_t* pstate) {

    ENTER;

    assert(pstate != NULL);
    ast_grammar_t* ptr = NULL;

    int state = 100;
    bool finished = false;

    int post = post_token_queue();

    ast_node_t* rule = NULL;
    pointer_list_t* list = NULL;

    while(!finished) {
        switch(state) {
            case 100:
                TRACE;
                if(NULL != (rule = (ast_node_t*)parse_non_terminal_rule(pstate))) {
                    list = create_pointer_list();
                    add_pointer_list(list, rule);
                    state = 110;
                }
                else if(NULL != (rule = (ast_node_t*)parse_terminal_rule(pstate))) {
                    list = create_pointer_list();
                    add_pointer_list(list, rule);
                    state = 110;
                }
                else {
                    PARSE_ERROR("grammar must contain at least one rule");
                    state = ERROR_STATE;
                }
                break;

            case 110:
                TRACE;
                if(NULL != (rule = (ast_node_t*)parse_non_terminal_rule(pstate)))
                    add_pointer_list(list, rule);
                else if(NULL != (rule = (ast_node_t*)parse_terminal_rule(pstate)))
                    add_pointer_list(list, rule);
                else
                    state = 120;
                break;

            case 120:
                TRACE;
                if(TTYPE == END_OF_INPUT) {
                    consume_token();
                    state = MATCH_STATE;
                }
                else {
                    EXPECTED("end of input");
                    state = ERROR_STATE;
                }
                break;

            case MATCH_STATE:
                TRACE;
                ptr = (ast_grammar_t*)create_ast_node(AST_GRAMMAR);
                ptr->rules = list;
                finished = true;
                break;

            case NO_MATCH_STATE:
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case ERROR_STATE:
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("unknown state in %s: %d\n", __func__, state);
        }
    }

    RETURN(ptr);
}

/*
 * terminal_rule {
 *     TERMINAL_SYMBOL TERMINAL_EXPR
 * }
 *
 */
static ast_terminal_rule_t* parse_terminal_rule(parser_state_t* pstate) {
    ENTER;

    assert(pstate != NULL);
    ast_terminal_rule_t* ptr = NULL;

    int state = 100;
    bool finished = false;

    int post = post_token_queue();

    token_t* term_sym = NULL;
    token_t* term_expr = NULL;

    while(!finished) {
        switch(state) {
            case 100:
                TRACE;
                if(TTYPE == TERMINAL_SYMBOL) {
                    term_sym = get_token();
                    consume_token();
                    state = 110;
                }
                else {
                    // EXPECTED("a non-terminal symbol");
                    state = NO_MATCH_STATE;
                }
                break;

            case 110:
                TRACE;
                if(TTYPE == TERMINAL_EXPR) {
                    term_expr = get_token();
                    consume_token();
                    state = MATCH_STATE;
                }
                else {
                    EXPECTED("a lexical expression");
                    state = ERROR_STATE;
                }
                break;

            case MATCH_STATE:
                TRACE;
                ptr = (ast_terminal_rule_t*)create_ast_node(AST_TERMINAL_RULE);
                ptr->term_sym = term_sym;
                ptr->term_expr = term_expr;
                finished = true;
                break;

            case NO_MATCH_STATE:
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case ERROR_STATE:
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("unknown state in %s: %d\n", __func__, state);
        }
    }

    RETURN(ptr);
}


/*
 * non_terminal_rule {
 *     NON_TERMINAL '{' +rule_element '}'
 * }
 *
 */
static ast_non_terminal_rule_t* parse_non_terminal_rule(parser_state_t* pstate) {

    ENTER;

    assert(pstate != NULL);
    ast_non_terminal_rule_t* ptr = NULL;

    int state = 100;
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
                    // EXPECTED("a non-terminal symbol");
                    state = NO_MATCH_STATE;
                }
                break;

            case 110:
                TRACE;
                if(TTYPE == OCURLY) {
                    consume_token();
                    state = 120;
                }
                else {
                    EXPECTED("a \"{\"");
                    state = ERROR_STATE;
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
                    PARSE_ERROR(
                            "at least one rule element is required in a rule");
                    state = ERROR_STATE;
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
                if(TTYPE == CCURLY) {
                    consume_token();
                    state = MATCH_STATE;
                }
                else {
                    EXPECTED("a \"}\"");
                    state = ERROR_STATE;
                }
                break;

            case MATCH_STATE:
                TRACE;
                ptr = (ast_non_terminal_rule_t*)create_ast_node(AST_NON_TERMINAL_RULE);
                ptr->nterm = nterm;
                ptr->rule_elems = rule_elems;
                finished = true;
                break;

            case NO_MATCH_STATE:
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case ERROR_STATE:
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("unknown state in %s: %d\n", __func__, state);
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

    int state = 100;
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
                    consume_token();
                    state = MATCH_STATE;
                }
                else
                    state = 200;
                break;

            case 200:
                TRACE;
                if(TTYPE == TERMINAL_NAME) {
                    term = get_token();
                    consume_token();
                    state = MATCH_STATE;
                }
                else
                    state = 300;
                break;

            case 300:
                TRACE;
                if(TTYPE == TERMINAL_OPER) {
                    term = get_token();
                    consume_token();
                    state = MATCH_STATE;
                }
                else
                    state = 400;
                break;

            case 400:
                TRACE;
                if(TTYPE == TERMINAL_SYMBOL) {
                    term = get_token();
                    consume_token();
                    state = MATCH_STATE;
                }
                else
                    state = 500;
                break;

            case 500:
                TRACE;
                if(NULL != (nterm = (ast_node_t*)parse_or_func(pstate)))
                    state = MATCH_STATE;
                else
                    state = 600;
                break;

            case 600:
                TRACE;
                if(NULL != (nterm = (ast_node_t*)parse_zero_or_more_func(pstate)))
                    state = MATCH_STATE;
                else
                    state = 700;
                break;

            case 700:
                TRACE;
                if(NULL != (nterm = (ast_node_t*)parse_zero_or_one_func(pstate)))
                    state = MATCH_STATE;
                else
                    state = 800;
                break;

            case 800:
                TRACE;
                if(NULL != (nterm = (ast_node_t*)parse_one_or_more_func(pstate)))
                    state = MATCH_STATE;
                else
                    state = 900;
                break;

            case 900:
                TRACE;
                if(NULL != (nterm = (ast_node_t*)parse_group_func(pstate)))
                    state = MATCH_STATE;
                else
                    // EXPECTED("a function or a terminal");
                    state = NO_MATCH_STATE;
                //}
                break;

            case MATCH_STATE:
                TRACE;
                ptr = (ast_rule_element_t*)create_ast_node(AST_RULE_ELEMENT);
                ptr->term = term;
                ptr->nterm = nterm;
                finished = true;
                break;

            case NO_MATCH_STATE:
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case ERROR_STATE:
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("unknown state in %s: %d\n", __func__, state);
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

    int state = 100;
    bool finished = false;

    int post = post_token_queue();
    ast_rule_element_t* re = NULL;

    while(!finished) {
        switch(state) {
            case 100:
                TRACE;
                if(TTYPE == ONE_OR_MORE) {
                    consume_token();
                    state = 110;
                }
                else
                    state = NO_MATCH_STATE;
                break;

            case 110:
                TRACE;
                if(NULL != (re = parse_rule_element(pstate)))
                    state = MATCH_STATE;
                else {
                    EXPECTED("one or more rule elements");
                    state = ERROR_STATE;
                }
                break;

            case MATCH_STATE:
                TRACE;
                ptr = (ast_one_or_more_func_t*)create_ast_node(AST_ONE_OR_MORE_FUNC);
                ptr->elem = re;
                finished = true;
                break;

            case NO_MATCH_STATE:
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case ERROR_STATE:
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("unknown state in %s: %d\n", __func__, state);
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

    int state = 100;
    bool finished = false;

    int post = post_token_queue();
    ast_rule_element_t* re = NULL;

    while(!finished) {
        switch(state) {
            case 100:
                TRACE;
                if(TTYPE == ZERO_OR_ONE) {
                    consume_token();
                    state = 110;
                }
                else
                    state = NO_MATCH_STATE;
                break;

            case 110:
                TRACE;
                if(NULL != (re = parse_rule_element(pstate)))
                    state = MATCH_STATE;
                else {
                    EXPECTED("one or more rule elements");
                    state = ERROR_STATE;
                }
                break;

            case MATCH_STATE:
                TRACE;
                ptr = (ast_zero_or_one_func_t*)create_ast_node(AST_ZERO_OR_ONE_FUNC);
                ptr->elem = re;
                finished = true;
                break;

            case NO_MATCH_STATE:
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case ERROR_STATE:
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("unknown state in %s: %d\n", __func__, state);
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

    int state = 100;
    bool finished = false;

    int post = post_token_queue();
    ast_rule_element_t* re = NULL;

    while(!finished) {
        switch(state) {
            case 100:
                TRACE;
                if(TTYPE == ZERO_OR_MORE) {
                    consume_token();
                    state = 110;
                }
                else
                    state = NO_MATCH_STATE;
                break;

            case 110:
                TRACE;
                if(NULL != (re = parse_rule_element(pstate)))
                    state = MATCH_STATE;
                else {
                    EXPECTED("one or more rule elements");
                    state = ERROR_STATE;
                }
                break;

            case MATCH_STATE:
                TRACE;
                ptr = (ast_zero_or_more_func_t*)create_ast_node(AST_ZERO_OR_MORE_FUNC);
                ptr->elem = re;
                finished = true;
                break;

            case NO_MATCH_STATE:
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case ERROR_STATE:
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("unknown state in %s: %d\n", __func__, state);
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

    int state = 100;
    bool finished = false;

    int post = post_token_queue();
    ast_rule_element_t* re = NULL;

    while(!finished) {
        switch(state) {
            case 100:
                TRACE;
                if(TTYPE == PIPE) {
                    consume_token();
                    state = 110;
                }
                else
                    state = NO_MATCH_STATE;
                break;

            case 110:
                TRACE;
                if(NULL != (re = parse_rule_element(pstate)))
                    state = MATCH_STATE;
                else {
                    EXPECTED("one or more rule elements");
                    state = ERROR_STATE;
                }
                break;

            case MATCH_STATE:
                TRACE;
                ptr = (ast_or_func_t*)create_ast_node(AST_OR_FUNC);
                ptr->elem = re;
                finished = true;
                break;

            case NO_MATCH_STATE:
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case ERROR_STATE:
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("unknown state in %s: %d\n", __func__, state);
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

    int state = 100;
    bool finished = false;

    int post = post_token_queue();
    pointer_list_t* list = NULL;
    ast_rule_element_t* re = NULL;

    while(!finished) {
        switch(state) {
            case 100:
                TRACE;
                if(TTYPE == OPAREN) {
                    consume_token();
                    state = 110;
                }
                else
                    state = NO_MATCH_STATE;
                break;

            case 110:
                TRACE;
                if(NULL != (re = parse_rule_element(pstate))) {
                    list = create_pointer_list();
                    add_pointer_list(list, re);
                    state = 120;
                }
                else {
                    EXPECTED("one or more rule elements");
                    state = ERROR_STATE;
                }
                break;

            case 120:
                TRACE;
                if(NULL != (re = parse_rule_element(pstate)))
                    add_pointer_list(list, re);
                else
                    state = 130;
                break;

            case 130:
                if(TTYPE == CPAREN) {
                    consume_token();
                    state = MATCH_STATE;
                }
                else {
                    EXPECTED("a \")\"");
                    state = ERROR_STATE;
                }
                break;

            case MATCH_STATE:
                TRACE;
                ptr = (ast_group_func_t*)create_ast_node(AST_GROUP_FUNC);
                ptr->list = list;
                finished = true;
                break;

            case NO_MATCH_STATE:
                TRACE;
                reset_token_queue(post);
                finished = true;
                break;

            case ERROR_STATE:
                TRACE;
                finished = true;
                break;

            default:
                fatal_error("unknown state in %s: %d\n", __func__, state);
        }
    }

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

    START;

    assert(file_name != NULL);
    void* ptr = parse_grammar(init_parser(file_name));

    FINISH(ptr);
}
