
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "parser.h"
#include "scanner.h"

typedef struct {
    token_t** list;
    int cap;
    int len;
    int crnt;
} scanner_t;

static scanner_t* scanner = NULL;

static void add_token(token_type_t type, const char* str);

%}

%option yylineno
%option noinput
%option noyywrap
%option header-file="scan.gen.h"
%option outfile="scanner.c"

%%

"|"     {add_token(PIPE, yytext); return PIPE;}
"+"     {add_token(ONE_OR_MORE, yytext); return ONE_OR_MORE;}
"*"     {add_token(ZERO_OR_MORE, yytext); return ZERO_OR_MORE;}
"?"     {add_token(ZERO_OR_ONE, yytext); return ZERO_OR_ONE;}
"("     {add_token(OPAREN, yytext); return OPAREN;}
")"     {add_token(CPAREN, yytext); return CPAREN;}
"{"     {add_token(OCURLY, yytext); return OCURLY;}
"}"     {add_token(CCURLY, yytext); return CCURLY;}

[A-Z_]+ {
        add_token(TERMINAL_SYMBOL, yytext);
        return TERMINAL_SYMBOL;
    }

[a-zA-Z_]+  {
        add_token(NON_TERMINAL, yytext);
        return NON_TERMINAL;
    }

\'[a-zA-Z_]+\' {
        add_token(TERMINAL_NAME, yytext);
        return TERMINAL_NAME;
    }

\'[^a-zA-Z_\']+\' {
        add_token(TERMINAL_OPER, yytext);
        return TERMINAL_OPER;
    }

\"[a-zA-Z_]+\" {
        add_token(TERMINAL_NAME, yytext);
        return TERMINAL_NAME;
    }

#[^\n]*\n  { /* comment */ }

[ \t\n\r]+ { /* ignore spaces */ }

. {
        printf("Error at %d: unrecognized character \"%s\"\n", yylineno, yytext);
        exit(0);
    }

%%

char* _str_dup(const char* str) {

    int len = strlen(str)+1;
    char* ptr = malloc(len);
    memcpy(ptr, str, len);
    return ptr;
}

static char* decorate_nterm(const char* str) {

    const char* finish = "_TOKEN";
    static char tmp_buf[64];
    memset(tmp_buf, 0, sizeof(tmp_buf));

    for(int i = 0; str[i] != '\0'; i++) {

        tmp_buf[i] = toupper(str[i]);

        if(i+strlen(finish)+1 > sizeof(tmp_buf)) {
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

    for(int i = 1; str[i+1] != '\0'; i++) {

        tmp_buf[i-1] = toupper(str[i]);

        if(i+strlen(finish)+1 > sizeof(tmp_buf)) {
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

    for(int i = 1; str[i+1] != '\0'; i++) {

        switch(str[i]) {
            case '~': strcat(tmp_buf, "TILDE_"); break;
            case '`': strcat(tmp_buf, "BQUOTE_"); break;
            case '!': strcat(tmp_buf, "BANG_"); break;
            case '@': strcat(tmp_buf, "AT_"); break;
            case '#': strcat(tmp_buf, "POUND_"); break;
            case '$': strcat(tmp_buf, "DOLLAR_"); break;
            case '%': strcat(tmp_buf, "PRECENT_"); break;
            case '^': strcat(tmp_buf, "CARAT_"); break;
            case '&': strcat(tmp_buf, "AMPERSAND_"); break;
            case '*': strcat(tmp_buf, "STAR_"); break;
            case '(': strcat(tmp_buf, "OPAREN_"); break;
            case ')': strcat(tmp_buf, "CPAREN_"); break;
            case '-': strcat(tmp_buf, "MINUS_"); break;
            case '+': strcat(tmp_buf, "PLUS_"); break;
            case '=': strcat(tmp_buf, "EQUAL_"); break;
            case '{': strcat(tmp_buf, "OCBRACE_"); break;
            case '[': strcat(tmp_buf, "OSBRACE_"); break;
            case '}': strcat(tmp_buf, "CCBRACE_"); break;
            case ']': strcat(tmp_buf, "CSBRACE_"); break;
            case ':': strcat(tmp_buf, "COLON_"); break;
            case ';': strcat(tmp_buf, "SCOLON_"); break;
            case '\"': strcat(tmp_buf, "DQUOTE_"); break;
            case '\'': strcat(tmp_buf, "SQUOTE_"); break;
            case '<': strcat(tmp_buf, "OPBRACE_"); break;
            case ',': strcat(tmp_buf, "COMMA_"); break;
            case '>': strcat(tmp_buf, "CPBRACE_"); break;
            case '.': strcat(tmp_buf, "DOT_"); break;
            case '?': strcat(tmp_buf, "QUESTION_"); break;
            case '/': strcat(tmp_buf, "SLASH_"); break;
            case '\\': strcat(tmp_buf, "BSLASH_"); break;
            case '|': strcat(tmp_buf, "BAR_"); break;
            default:
                tmp_buf[strlen(tmp_buf)] = toupper(str[i]);
                tmp_buf[strlen(tmp_buf)] = '_';
                break;

        }

        if(i+strlen(finish)+1 > sizeof(tmp_buf)) {
            fprintf(stderr, "FATAL: convert exceeds size of tmp_buf\n");
            fprintf(stderr, "on line number %d\n", yylineno);
            exit(1);
        }
    }

    strcat(tmp_buf, finish);

    return tmp_buf;
}

static void add_token(token_type_t type, const char* str) {

    token_t* tok = malloc(sizeof(token_t));
    tok->line_no = yylineno;
    tok->type = type;
    tok->text = _str_dup(str);

    if(scanner->len+1 > scanner->cap) {
        scanner->cap <<= 1;
        scanner->list = realloc(scanner->list, scanner->cap*sizeof(token_t*));
    }

    scanner->list[scanner->len] = tok;
    scanner->len++;

    if(tok->type == NON_TERMINAL) {
        tok->name = _str_dup(decorate_nterm(tok->text));
    }
    else if(tok->type == TERMINAL_NAME) {
        tok->name = _str_dup(decorate_term_name(tok->text));
    }
    else if(tok->type == TERMINAL_OPER) {
        tok->name = _str_dup(decorate_term_oper(tok->text));
    }
    else
        tok->name = NULL;
}

void init_scanner(const char* file_name) {

    yyin = fopen(file_name, "r");
    if (yyin == NULL){
        printf("cannot open input file: %s: %s\n", file_name, strerror(errno));
        exit(1);
    }

    scanner = malloc(sizeof(scanner_t));
    scanner->cap = 1 << 3;
    scanner->len = 0;
    scanner->crnt = 0;
    scanner->list = malloc(sizeof(token_t*)*scanner->cap);

    while(yylex()) {
        /* state driven execution */
        //printf("%s\n", yytext);
    }

    add_token(END_OF_INPUT, "");

    //printf("%d tokens read\n", scanner->len);
}

token_t* get_token(void) {

    return scanner->list[scanner->crnt];
}

token_t* advance_token(void) {

    if(scanner->crnt < scanner->len)
        scanner->crnt++;

    return get_token();
}

int post_token_queue(void) {

    return scanner->crnt;
}

void reset_token_queue(int post) {

    scanner->crnt = post;
}

const char* tok_type_to_str(token_t* tok) {

    return
        (tok->type == END_OF_INPUT)? "END_OF_INPUT" :
        (tok->type == PIPE)? "PIPE" :
        (tok->type == ONE_OR_MORE)? "ONE_OR_MORE" :
        (tok->type == ZERO_OR_MORE)? "ZERO_OR_MORE" :
        (tok->type == ZERO_OR_ONE)? "ZERO_OR_ONE" :
        (tok->type == OPAREN)? "OPAREN" :
        (tok->type == CPAREN)? "CPAREN" :
        (tok->type == OCURLY)? "OCURLY" :
        (tok->type == CCURLY)? "CCURLY" :
        (tok->type == NON_TERMINAL)? "NON_TERMINAL" :
        (tok->type == TERMINAL_SYMBOL)? "TERMINAL_SYMBOL" :
        (tok->type == TERMINAL_NAME)? "TERMINAL_NAME" :
        (tok->type == TERMINAL_OPER)? "TERMINAL_OPER" :
        (tok->type == TERMINAL)? "TERMINAL" : "UNKNOWN";
}

const char* tok_to_str(token_t* tok) {

    return tok->text;
}

int get_line_no(void) {
    return yylineno;
}
