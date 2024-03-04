#ifndef TYPE_H_INCLUDED
#define TYPE_H_INCLUDED



#if 1&&0



#define TYPE_ARRAY_NONE SIZE_MAX
#define TYPE_ARRAY_INDETERMINATE (SIZE_MAX - 1)

typedef void type_t;

//type_t* type_new_basic(base_t base, size_t indirection_count, size_t array_count);

//type_t* type_new_record(record_t* record, size_t indirection_count, size_t array_count);

void type_delete(type_t* type);

type_t* type_clone(type_t* other);

base_t type_base(type_t* type);

void type_set_base(type_t* type, base_t base);

record_t* type_record(type_t* type);

void type_set_record(type_t* type, record_t* record);

bool type_is_lvalue(type_t* type);

void type_set_lvalue(type_t* type, bool lvalue);

/**
 * Returns the indirection count, i.e. the number of pointers.
 */
int type_indirections(type_t* type);

void type_set_indirections(type_t* type, int count);

/**
 * Returns the array length, or TYPE_ARRAY_NONE if it is not an array, or
 * TYPE_ARRAY_INDETERMINATE if it is an array of indeterminate length.
 */
int type_array_length(type_t* type);

void type_set_array_length(type_t* type, int array_length);

size_t type_size(type_t type);
#endif










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
 * We don't have sizeof so this is sizeof(type_t).
 */
#define TYPE_T_SIZE 1

/**
 * Makes a type.
 */
type_t type_make(int base, int indirection);

/**
 * Returns the size of a type.
 */
int type_size(type_t type);

/**
 * Returns the basic type of the given type_t.
 */
int type_basic(type_t type);

/**
 * Returns the indirection count of the given type_t.
 */
int type_indirection(type_t type);

/**
 * Decrements the indirection count of the given type, keeping the rest the
 * same.
 */
type_t type_decrement_indirection(type_t type);




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
 * This takes ownership of the given name.
 */
void typedef_add(char* name, type_t type);

int typedef_find(const char* name, type_t* out_type);

#endif
