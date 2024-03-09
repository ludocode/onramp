#ifndef PARSE_EXPR_H_INCLUDED
#define PARSE_EXPR_H_INCLUDED

#include <stdbool.h>

#include "type.h"

/**
 * Tries to parse a constant expression.
 */
bool try_parse_constant_expression(type_t** type, int* value);

#endif
