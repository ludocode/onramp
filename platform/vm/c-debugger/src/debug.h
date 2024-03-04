#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#include "vmcommon.h"

void debug_init(void);
void debug_destroy(void);

/**
 * Loads debug info (if it exists) for the given executable loaded at the given
 * address.
 */
void debug_load(const char* executable_filename, size_t address);

/**
 * Finds the source filename and line location for a given code address in
 * memory.
 */
bool debug_find(size_t address, const char** symbol, const char** out_filename, int* out_line);

/**
 * Pushes a new stack frame.
 */
void debug_callstack_push(uint32_t source_address, uint32_t return_address, bool tail_call);

/**
 * Pops all stack frames from the top of the stack that have the given return
 * address.
 */
void debug_callstack_pop(uint32_t return_address);

void debug_callstack_print(uint32_t current_address);

/**
 * Returns true if the top of the stack has the given return address.
 */
bool debug_stack_has_return(uint32_t return_address);

// TODO remove
void debug_callstack_print_line(uint32_t address, bool tail_call);

bool debug_print_location(uint32_t address, bool hide_path);

#endif
