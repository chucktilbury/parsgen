/*
 * This is a AST pass that outputs the grammar input after it has been
 * processed. It is used to verify that the input was properly parsed and that
 * the AST is traversing correctly. It also serves as a template for parser
 * implementations.
 */
#include <stdio.h>

#include "ast.h"
#include "errors.h"
#include "memory.h"
#include "regurge.h"

static FILE* fh = NULL;

/*
 * This function is entered before the node is traversed.
 */
static void regurge_pre(ast_node_t* node) {

    switch(node->type) {
        case AST_GRAMMAR:
            break;
        case AST_RULE:
            fprintf(fh, "%s {\n        ", ((ast_rule_t*)node)->nterm->text);
            break;
        case AST_RULE_ELEMENT: {
            ast_rule_element_t* elem = (ast_rule_element_t*)node;
            if(elem->term != NULL) {
                fprintf(fh, "%s ", elem->term->text ? elem->term->text : elem->term->name);
            }
        } break;
        case AST_ONE_OR_MORE_FUNC:
            fprintf(fh, "+ ");
            break;
        case AST_ZERO_OR_ONE_FUNC:
            fprintf(fh, "? ");
            break;
        case AST_ZERO_OR_MORE_FUNC:
            fprintf(fh, "* ");
            break;
        case AST_OR_FUNC:
            fprintf(fh, "|\n        ");
            break;
        case AST_GROUP_FUNC:
            fprintf(fh, "( ");
            break;
        default:
            fatal_error("unknown state in %s", __func__);
    }
}

/*
 * This function is entered after the node is traversed.
 */
static void regurge_post(ast_node_t* node) {

    switch(node->type) {
        case AST_GRAMMAR:
            break;
        case AST_RULE:
            fprintf(fh, "\n    }\n\n");
            break;
        case AST_RULE_ELEMENT:
            break;
        case AST_ONE_OR_MORE_FUNC:
            break;
        case AST_ZERO_OR_ONE_FUNC:
            break;
        case AST_ZERO_OR_MORE_FUNC:
            break;
        case AST_OR_FUNC:
            break;
        case AST_GROUP_FUNC:
            fprintf(fh, ") ");
            break;
        default:
            fatal_error("unknown state in %s", __func__);
    }
}

/*
 * Public interface
 */
void ast_regurge(void* ptr) {

    ast_state_t* state = _ALLOC_DS(ast_state_t);
    state->pre = (ast_callback_t)regurge_pre;
    state->post = (ast_callback_t)regurge_post;

    fh = stdout;

    traverse_ast(ptr, state);

    _FREE(state);
}
