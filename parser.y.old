
%{
 #include <stdio.h>
 #include <stdlib.h>
 #include "scanner.h"

 void yyerror(const char *msg);

%}

%union{
    char* name;
    char* tsym;
    char* term;
}

%define parse.error verbose
%locations

%token PIPE ONE_OR_MORE ZERO_OR_MORE ZERO_OR_ONE
%token OPAREN CPAREN OCURLY CCURLY
%token <name> NAME
%token <tsym> TERMINAL_SYMBOL
%token <term> TERMINAL

%left ONE_OR_MORE ZERO_OR_MORE ZERO_OR_ONE
%right PIPE

%%

grammar
    : {
            printf("initialize system\n");
        }
        rule_list {
            printf("finished parsing grammar\n");
        }
    ;

rule_list
    : rule_item  {
            printf("rule_list:start\n");
        }
    | rule_list rule_item {
            printf("rule_list:add\n");
        }
    ;

rule_item
    : NAME {
            printf("** beginning of a new rule: %s\n", $1);
        }
    OCURLY expression CCURLY  {
            printf("** finished a complete rule\n");
        }
    ;

rule_element
    : NAME {
            printf("rule_element:NAME %s\n", $1);
        }
    | TERMINAL_SYMBOL {
            printf("rule_element:TERMINAL_SYMBOL %s\n", $1);
        }
    | TERMINAL {
            printf("rule_element:TERMINAL %s\n", $1);
        }
    ;

rule_element_list
    : rule_element {
            printf("rule_element_list:start\n");
        }
    | rule_element_list rule_element {
            printf("rule_element_list:add\n");
        }
    ;

expr_primary
    : rule_element_list {
            printf("expr_primary:rule_element_list\n");
        }
    ;

expression
    : expr_primary {
            printf("expression:rule_element_list\n");
        }
    | expression ONE_OR_MORE {
            printf("expression:ONE_OR_MORE\n");
        }
    | expression ZERO_OR_MORE {
            printf("expression:ZERO_OR_MORE\n");
        }
    | expression ZERO_OR_ONE {
            printf("expression:ZERO_OR_ONE\n");
        }
    | expression PIPE expression {
            printf("expression:PIPE:expression\n");
        }
    | OPAREN expression CPAREN {
            printf("expr_primary:(expression)\n");
        }
    ;


%%

void yyerror(const char *msg) {

   printf("** Line %d: %s\n", yylineno, msg);
}
