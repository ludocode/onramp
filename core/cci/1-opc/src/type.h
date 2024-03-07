#ifndef TYPE_H_INCLUDED
#define TYPE_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>

#include "common.h"

/**
 * A base type.
 *
 * Note: `char`, `signed char` and `unsigned char` are technically supposed to
 * be three distinct types in C. `int` and `long` are also supposed to be
 * distinct. This matters for `_Generic` among other things, but we don't
 * support that in opC so we don't bother differentating between these. In opC,
 * `char` and `signed char` are the same thing, and `long` is treated as `int`
 * after parsing.
 */
typedef int base_t;

#define BASE_RECORD 01
#define BASE_VOID 02
#define BASE_UNSIGNED_CHAR 03
#define BASE_UNSIGNED_SHORT 04
#define BASE_UNSIGNED_INT 05
#define BASE_UNSIGNED_LONG_LONG 06
#define BASE_SIGNED_CHAR 13
#define BASE_SIGNED_SHORT 14
#define BASE_SIGNED_INT 15
#define BASE_SIGNED_LONG_LONG 16

#define TYPE_ARRAY_NONE SIZE_MAX
#define TYPE_ARRAY_INDETERMINATE (SIZE_MAX - 1)

/**
 * A type.
 *
 * A type in opC is described by:
 *
 * - a base primitive or record type
 * - an indirection count (number of pointers)
 * - an optional array size, which can be indeterminate
 * - an l-value flag (describing its state of compilation)
 */
typedef void type_t;

/**
 * Deletes a type.
 */
void type_delete(type_t* type);

/**
 * Creates a new type with the given primitive base.
 *
 * This cannot be BASE_RECORD; call type_new_record() instead.
 */
type_t* type_new_base(base_t base);

//type_t* type_new_record(record_t* record);

/**
 * Allocate a new copy of a type.
 */
type_t* type_clone(const type_t* other);

/**
 * Returns the size of a type.
 */
size_t type_size(const type_t* type);

/**
 * Returns the base type of the given type_t.
 */
base_t type_base(const type_t* type);

bool type_is_record(const type_t* type);

void type_set_base(type_t* type, base_t base);

/*
record_t* type_record(type_t* type);

void type_set_record(type_t* type, record_t* record);
*/

/**
 * Returns the indirection count of the given type_t.
 *
 * This includes an indirection for the array size, if it's an array.
 */
int type_indirections(const type_t* type);

void type_set_indirections(type_t* type, int count);

/**
 * Decrements the indirection count of the given type, keeping the rest the
 * same.
 */
type_t* type_decrement_indirection(type_t* type);

type_t* type_increment_indirection(type_t* type);

bool type_is_lvalue(const type_t* type);

type_t* type_set_lvalue(type_t* type, bool lvalue);

/**
 * Returns the array length, or TYPE_ARRAY_NONE if it is not an array, or
 * TYPE_ARRAY_INDETERMINATE if it is an array of indeterminate length.
 */
size_t type_array_length(const type_t* type);

void type_set_array_length(type_t* type, size_t array_length);

bool type_equal(const type_t* left, const type_t* right);

/**
 * Returns true if a type exactly matches the given base with no pointers,
 * array, record, etc.
 */
bool type_is_base(const type_t* type, base_t base);

void type_print(const type_t* type);



void typedef_init(void);
void typedef_destroy(void);

/**
 * Adds a new typedef.
 *
 * This takes ownership of the given name and type.
 */
void typedef_add(char* name, type_t* type);

const type_t* typedef_find(const char* name);

#endif
