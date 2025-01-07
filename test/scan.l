%{

#include <stdio.h>


%}

%option yylineno
%option noinput
%option noyywrap
%option header-file="scan.h"
%option outfile="scan.c"

%%

\"[^\n]+\" { printf("scan: %s\n", yytext); }

[ \t\n\r]+ { /* ignore spaces */ }

. {}

%%

int main() {

    yyin = fopen("input.txt", "r");

    while(yylex()) {}

    return 0;
}
