#ifndef PARSE_STMT_H_INCLUDED
#define PARSE_STMT_H_INCLUDED

void parse_stmt_init(void);
void parse_stmt_destroy(void);

/**
 * Stores a string literal to be emitted as a global after the current function
 * is done, returning its id.
 *
 * This consumes the current token, which must be a string literal.
 */
int store_string_literal(void);

/**
 * Parses a global variable or function, compiling it to the output.
 */
void parse_global(void);

#endif
