#ifndef LOCALS_H_INCLUDED
#define LOCALS_H_INCLUDED

#include "type.h"

#include <stdbool.h>

/**
 * The total number of variables in all currently accessible scopes.
 */
extern int locals_count;

void locals_init(void);
void locals_destroy(void);

/**
 * Adds a new variable with the given name and type.
 *
 * This takes ownership of the given name. It will be freed when locals_pop()
 * is called.
 */
void locals_add(char* name, type_t* type);

/**
 * Destroys all variables beyond the given variable count.
 */
void locals_pop(int previous_locals_count);

/**
 * Finds a variable, returning its type and offset.
 */
bool locals_find(const char* name, const type_t** type, int* offset);

/**
 * Returns the total size of all local variables currently defined, i.e. the
 * necessary stack frame size to store them.
 */
int locals_frame_size(void);

/**
 * Prints all variables.
 */
void dump_variables(void);

#endif
