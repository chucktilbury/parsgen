/*
 * This module implements the emitter phase of the parser generator. There are
 * several passes that build up the data structures to facilitate the output.
 *
 * Generate lists of the terminal and non-termial symbols.
 * Generate the list of comments. These are a text representation of the rules
 *      to facilitate debugging if required.
 * Generate the rule list. A list of data structures.
 *      Each rule has an item list. This allows the data structures to be
 *          correctly generated.
 */

#include <stdio.h>

#include "ast.h"
#include "errors.h"
#include "memory.h"
#include "regurge.h"
