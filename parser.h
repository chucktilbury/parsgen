
#ifndef _PARSER_H_
#define _PARSER_H_

#include "pointer_list.h"

// A rule list is the list of complete rules that was defined by the input.
typedef pointer_list_t rule_list_t;

// A func_list is the rule part of the rule. It is a list of nested functions
// that implement the rule.
typedef pointer_list_t func_list_t;

//
// A func_list_t contains these objects.
//
typedef struct {
    // This is the non-terminal name
    token_t* non_term_name;
    // A rule line list item is a list of rule functions that is separated by
    // the "OR" operator. (a list of lists)
    func_list_t* funcitons;
} rule_t;

typedef enum {
    EXACTLY_ONE,
    ONE_OR_MORE,
    ZERO_OR_MORE,
    ZERO_OR_ONE,
} func_type_t;

//
// Support for casting the object and getting the type.
//
struct function_type {
    func_type_t type;
};

//
// A function is an element of the rule_line_list_t. The function_list_t
// contains one or more functions. If the names parsed in the rule appear
// outside of parens, or if several names appear inside parens with no operator
// then it's an exactly one function type. Other function types can appear
// nested inside of any other function.
//
// For example:
// name1 (name2 name3)+
// will be parsed as
// EXACTLY_ONE(name1, ONE_OR_MORE(name2, name3))
//
// Note that function operators must appear after a ')' or it's a syntax error.
// The EXACTLY_ONE operator is implicit and does not require a ')'.
//
typedef struct {
    struct function_type type;
    func_list_t* funcs;
} function_t;

rule_list_t* parse(void);

#endif /* _PARSER_H_ */
