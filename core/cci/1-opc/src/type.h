#ifndef TYPE_H_INCLUDED
#define TYPE_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>

#include "common.h"
#include "base.h"

//type_t* type_new_basic(base_t base, size_t indirection_count, size_t array_count);

//type_t* type_new_record(record_t* record, size_t indirection_count, size_t array_count);



#define TYPE_ARRAY_NONE SIZE_MAX
#define TYPE_ARRAY_INDETERMINATE (SIZE_MAX - 1)

// TODO replace with base
#define TYPE_BASIC_VOID 0   // TODO why isn't this 0x10? could use 0 as invalid
#define TYPE_BASIC_CHAR 0x20
#define TYPE_BASIC_INT 0x40   // TODO this could be 0x30

#define TYPE_FLAG_LVALUE 0x80   // TODO this could be 0x40, would fit in mix-type arg

/**
 * A type is stored as a char.
 *
 * - Bits 0-4 are the indirection count.
 * - Bits 5-6 are the basic type.
 * - Bit 7 is an "lvalue" flag used in expression parsing.
 */
typedef char type_t;

/**
 * Allocates a new type.
 */
type_t* type_new_blank(void);

/**
 * Allocates a new type.
 */
type_t* type_new(int base, int indirection);

/**
 * Deletes a type.
 */
void type_delete(type_t* type);

/**
 * Allocate a new copy of a type.
 */
type_t* type_clone(const type_t* other);

/**
 * Returns the size of a type.
 */
int type_size(const type_t* type);

/**
 * Returns the base type of the given type_t.
 */
base_t type_base(const type_t* type);

void type_set_base(type_t* type, base_t base);

/*
record_t* type_record(type_t* type);

void type_set_record(type_t* type, record_t* record);
*/

/**
 * Returns the indirection count of the given type_t.
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
int type_array_length(type_t* type);

void type_set_array_length(type_t* type, int array_length);



/*
 * TODO replace "typedef" with "types", which will contain all basic types,
 * typedefs, structs and unions. A lookup into the types table specifies a name
 * and the tag (struct, union or none.)
 */

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
