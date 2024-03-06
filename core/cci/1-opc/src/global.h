#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#include "common.h"

void globals_init(void);
void globals_destroy(void);

typedef void global_t;

const char* global_name(const global_t* global);

const type_t* global_type(const global_t* global);

bool global_is_variable(const global_t* global);

bool global_is_function(const global_t* global);

/**
 * Returns true if the given global function is variadic.
 *
 * The given global must be a function.
 */
bool global_function_is_variadic(const global_t* global);

/**
 * Returns the number of (non-variadic) arguments of the given global.
 *
 * The given global must be a function.
 */
int global_function_arg_count(const global_t* global);

/**
 * Returns the number of (non-variadic) arguments of the given global.
 *
 * The given global must be a function.
 */
type_t* global_function_arg_type(const global_t* global, int index);

/**
 * Creates a new global variable, adding it to the globals table.
 *
 * This takes ownership of the given type and name.
 */
const global_t* global_declare_variable(type_t* type, char* name);

/**
 * Creates a new global function, adding it to the globals table.
 *
 * This takes ownership of the given type and name. If the global already
 * exists, the given values will be destroyed immediately! Use the returned
 * global's name and other properties after calling this.
 *
 * We can only take four arguments in omC so the variadic flag is set
 * separately in global_set_variadic().
 */
const global_t* global_declare_function(type_t* return_type, char* name,
        int arg_count, type_t** arg_types);

void global_set_variadic(global_t* global, bool variadic);

global_t* global_find(const char* name);

#endif
