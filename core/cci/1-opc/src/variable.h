#ifndef VARIABLE_H_INCLUDED
#define VARIABLE_H_INCLUDED

#include "type.h"

#include <stdbool.h>

/**
 * The total number of variables in all currently accessible scopes.
 */
extern int variable_count;

/**
 * The total number of global variables defined so far.
 */
extern int variable_global_count;

void variable_init(void);
void variable_destroy(void);

/**
 * Adds a new variable with the given name and type.
 *
 * This takes ownership of the given name. It will be freed when variable_pop()
 * is called.
 */
void variable_add(char* name, type_t type, bool global);

/**
 * Destroys all variables beyond the given variable count.
 */
void variable_pop(int previous_variable_count);

/**
 * Finds a variable, returning its type and offset.
 */
bool variable_find(const char* name, type_t* type, int* offset);

/**
 * Returns the total size of all local variables currently defined.
 */
int variable_local_size(void);

/**
 * Prints all variables.
 */
void dump_variables(void);

#endif
