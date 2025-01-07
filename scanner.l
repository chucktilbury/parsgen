
%{
#include "scanner.h"

%}

%option yylineno
%option noinput
%option noyywrap
%option header-file="scan.gen.h"
%option outfile="scanner.c"

%%
    /* These are part of the grammar syntax. */
"|"     {add_token(PIPE, yytext); return PIPE;}
"+"     {add_token(ONE_OR_MORE, yytext); return ONE_OR_MORE;}
"*"     {add_token(ZERO_OR_MORE, yytext); return ZERO_OR_MORE;}
"?"     {add_token(ZERO_OR_ONE, yytext); return ZERO_OR_ONE;}
"("     {add_token(OPAREN, yytext); return OPAREN;}
")"     {add_token(CPAREN, yytext); return CPAREN;}
"{"     {add_token(OCURLY, yytext); return OCURLY;}
"}"     {add_token(CCURLY, yytext); return CCURLY;}

[A-Z_][A-Z_0-9]* {
        add_token(TERMINAL_SYMBOL, yytext);
        return TERMINAL_SYMBOL;
    }

[a-zA-Z_][a-zA-Z_0-9]*  {
        add_token(NON_TERMINAL, yytext);
        return NON_TERMINAL;
    }

\'[a-zA-Z_][a-zA-Z_0-9]*\' {
        add_token(TERMINAL_NAME, yytext);
        return TERMINAL_NAME;
    }

\'[^a-zA-Z_\']+\' {
        add_token(TERMINAL_OPER, yytext);
        return TERMINAL_OPER;
    }

\"[^\n]+\" {
        add_token(TERMINAL_EXPR, yytext);
        return TERMINAL_EXPR;
    }

#[^\n]*\n  { /* comment */ }

[ \t\n\r]+ { /* ignore spaces */ }

. {
        fprintf(stderr, "Error at %d: unrecognized character \"%s\"\n", yylineno, yytext);
        exit(0);
    }

%%

