#ifndef _PARSER_H_
#define _PARSER_H_

/*
    grammar
    rule
    rule_element
    one_or_more_func
    zero_or_one_func
    zero_or_more_func
    or_func
    group_func
*/

typedef struct _parser_state_ {
    int state; // dummy
} parser_state_t;

void* parse(const char* file_name);

#endif /* _PARSER_H_ */
