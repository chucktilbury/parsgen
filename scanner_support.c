
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
// #include "parser.h"
#include "pointer_list.h"
#include "scan.gen.h"
#include "scanner.h"

typedef pointer_list_t scanner_t;
static scanner_t* scanner = NULL;
static int crnt = 0;
static const char* fname;

static char* decorate_nterm(const char* str) {

    const char* finish = "_TOKEN";
    static char tmp_buf[64];
    memset(tmp_buf, 0, sizeof(tmp_buf));

    for(int i = 0; str[i] != '\0'; i++) {

        tmp_buf[i] = toupper(str[i]);

        if(i + strlen(finish) + 1 > sizeof(tmp_buf)) {
            fprintf(stderr, "FATAL: convert exceeds size of tmp_buf\n");
            fprintf(stderr, "on line number %d\n", yylineno);
            exit(1);
        }
    }

    strcat(tmp_buf, finish);

    return tmp_buf;
}

static char* decorate_term_name(const char* str) {

    const char* finish = "_TOKEN";
    static char tmp_buf[64];
    memset(tmp_buf, 0, sizeof(tmp_buf));

    for(int i = 1; str[i + 1] != '\0'; i++) {

        tmp_buf[i - 1] = toupper(str[i]);

        if(i + strlen(finish) + 1 > sizeof(tmp_buf)) {
            fprintf(stderr, "FATAL: convert exceeds size of tmp_buf\n");
            fprintf(stderr, "on line number %d\n", yylineno);
            exit(1);
        }
    }

    strcat(tmp_buf, finish);

    return tmp_buf;
}

static char* decorate_term_oper(const char* str) {

    const char* finish = "TOKEN";
    static char tmp_buf[64];
    memset(tmp_buf, 0, sizeof(tmp_buf));

    for(int i = 1; str[i + 1] != '\0'; i++) {

        switch(str[i]) {
            case '~':
                strcat(tmp_buf, "TILDE_");
                break;
            case '`':
                strcat(tmp_buf, "BQUOTE_");
                break;
            case '!':
                strcat(tmp_buf, "BANG_");
                break;
            case '@':
                strcat(tmp_buf, "AT_");
                break;
            case '#':
                strcat(tmp_buf, "POUND_");
                break;
            case '$':
                strcat(tmp_buf, "DOLLAR_");
                break;
            case '%':
                strcat(tmp_buf, "PRECENT_");
                break;
            case '^':
                strcat(tmp_buf, "CARAT_");
                break;
            case '&':
                strcat(tmp_buf, "AMPERSAND_");
                break;
            case '*':
                strcat(tmp_buf, "STAR_");
                break;
            case '(':
                strcat(tmp_buf, "OPAREN_");
                break;
            case ')':
                strcat(tmp_buf, "CPAREN_");
                break;
            case '-':
                strcat(tmp_buf, "MINUS_");
                break;
            case '+':
                strcat(tmp_buf, "PLUS_");
                break;
            case '=':
                strcat(tmp_buf, "EQUAL_");
                break;
            case '{':
                strcat(tmp_buf, "OCBRACE_");
                break;
            case '[':
                strcat(tmp_buf, "OSBRACE_");
                break;
            case '}':
                strcat(tmp_buf, "CCBRACE_");
                break;
            case ']':
                strcat(tmp_buf, "CSBRACE_");
                break;
            case ':':
                strcat(tmp_buf, "COLON_");
                break;
            case ';':
                strcat(tmp_buf, "SCOLON_");
                break;
            case '\"':
                strcat(tmp_buf, "DQUOTE_");
                break;
            case '\'':
                strcat(tmp_buf, "SQUOTE_");
                break;
            case '<':
                strcat(tmp_buf, "OPBRACE_");
                break;
            case ',':
                strcat(tmp_buf, "COMMA_");
                break;
            case '>':
                strcat(tmp_buf, "CPBRACE_");
                break;
            case '.':
                strcat(tmp_buf, "DOT_");
                break;
            case '?':
                strcat(tmp_buf, "QUESTION_");
                break;
            case '/':
                strcat(tmp_buf, "SLASH_");
                break;
            case '\\':
                strcat(tmp_buf, "BSLASH_");
                break;
            case '|':
                strcat(tmp_buf, "BAR_");
                break;
            default:
                tmp_buf[strlen(tmp_buf)] = toupper(str[i]);
                tmp_buf[strlen(tmp_buf)] = '_';
                break;
        }

        if(i + strlen(finish) + 1 > sizeof(tmp_buf)) {
            fprintf(stderr, "FATAL: convert exceeds size of tmp_buf\n");
            fprintf(stderr, "on line number %d\n", yylineno);
            exit(1);
        }
    }

    strcat(tmp_buf, finish);

    return tmp_buf;
}

void add_token(token_type_t type, const char* str) {

    token_t* tok = _ALLOC_DS(token_t);
    tok->line_no = yylineno;
    tok->type = type;
    tok->text = _COPY_STRING(str);

    if(tok->type == NON_TERMINAL) {
        tok->name = _COPY_STRING(decorate_nterm(tok->text));
    }
    else if(tok->type == TERMINAL_NAME) {
        tok->name = _COPY_STRING(decorate_term_name(tok->text));
    }
    else if(tok->type == TERMINAL_OPER) {
        tok->name = _COPY_STRING(decorate_term_oper(tok->text));
    }
    else
        // TERMINAL_SYMBOL
        tok->name = NULL;

    add_pointer_list(scanner, tok);
}

void init_scanner(const char* file_name) {

    yyin = fopen(file_name, "r");
    if(yyin == NULL) {
        printf("cannot open input file: %s: %s\n", file_name, strerror(errno));
        exit(1);
    }

    fname = _COPY_STRING(file_name);
    scanner = create_pointer_list();

    while(yylex()) {
        /* state driven execution */
        // printf("%s\n", yytext);
    }

    add_token(END_OF_INPUT, "end of input");

    // printf("%d tokens read\n", scanner->len);

    // printf("cap: %d\n", scanner->cap);
    // printf("len: %d\n", scanner->len);
}

void uninit_scanner(void) {

    if(scanner != NULL) {
        int idx = 0;
        token_t* tok;
        while(NULL != (tok = iterate_pointer_list(scanner, &idx))) {
            _FREE(tok->text);
            _FREE(tok->name);
            _FREE(tok);
        }

        destroy_pointer_list(scanner);
    }
}

token_t* get_token(void) {

    return index_pointer_list(scanner, crnt);
}

token_t* consume_token(void) {

    // do not iterate past the end of the list to return NULL as
    // the iterator does.
    if(crnt + 1 < scanner->len)
        return iterate_pointer_list(scanner, &crnt);
    else
        return get_token();
}

int post_token_queue(void) {

    return crnt;
}

void reset_token_queue(int post) {

    crnt = post;
}

const char* tok_type_to_str(token_t* tok) {

    return (tok->type == END_OF_INPUT)     ? "END_OF_INPUT" :
            (tok->type == PIPE)            ? "PIPE" :
            (tok->type == ONE_OR_MORE)     ? "ONE_OR_MORE" :
            (tok->type == ZERO_OR_MORE)    ? "ZERO_OR_MORE" :
            (tok->type == ZERO_OR_ONE)     ? "ZERO_OR_ONE" :
            (tok->type == OPAREN)          ? "OPAREN" :
            (tok->type == CPAREN)          ? "CPAREN" :
            (tok->type == OCURLY)          ? "OCURLY" :
            (tok->type == CCURLY)          ? "CCURLY" :
            (tok->type == NON_TERMINAL)    ? "NON_TERMINAL" :
            (tok->type == TERMINAL_SYMBOL) ? "TERMINAL_SYMBOL" :
            (tok->type == TERMINAL_NAME)   ? "TERMINAL_NAME" :
            (tok->type == TERMINAL_OPER)   ? "TERMINAL_OPER" :
            (tok->type == TERMINAL_EXPR)   ? "TERMINAL_EXPR" :
                                             "UNKNOWN";
}

const char* tok_to_str(token_t* tok) {

    return tok->text;
}

int get_line_no(void) {
    return yylineno;
}

const char* get_file_name(void) {
    return fname;
}
