#include "type.h"

#include <stdlib.h>
#include <string.h>

#include "common.h"




// TODO new type class

#if 1&&0


/*
 * type_t looks something like this:
 *
 * typedef struct {
 *     basic_type_t base;
 *     record_t* record;
 *     char indirection_count;
 *     int array_length;
 *     bool is_lvalue;
 * } type_t;
 *
 * We make each field a void* for simplicity.
 */

#define TYPE_OFFSET_BASE 0
#define TYPE_OFFSET_RECORD 1
#define TYPE_OFFSET_INDIRECTION_COUNT 2
#define TYPE_OFFSET_ARRAY_SIZE 3
#define TYPE_OFFSET_IS_LVALUE 4
#define TYPE_FIELD_COUNT 5

static type_t* type_new(void) {
    type_t* type = calloc(TYPE_FIELD_COUNT, sizeof(void*));
    if (!type) {
        fatal("Out of memory.");
    }
    return type;
}

/*
type_t* type_new_basic(basic_type_t base, size_t indirection_count, size_t array_length) {
    type_t* type = type_new(indirection_count, array_length);
}

type_t* type_new_record(record_t* record, size_t indirection_count, size_t array_length) {
}
*/

void type_delete(type_t* type) {
    free(type);
}

type_t* type_clone(type_t* other) {
    type_t* type = memdup(other, TYPE_FIELD_COUNT * sizeof(void*));
    if (!type) {
        fatal("Out of memory.");
    }
    return type;
}

base_t type_base(type_t* type) {
    return (base_t)*((void**)type + TYPE_OFFSET_BASE);
}

void type_set_base(type_t* type, base_t base) {
    *((void**)type + TYPE_OFFSET_BASE) = (void*)base;
}

record_t* type_record(type_t* type) {
    return *((void**)type + TYPE_OFFSET_RECORD);
}

void type_set_record(type_t* type, record_t* record) {
    *((void**)type + TYPE_OFFSET_RECORD) = (void*)record;
}

bool type_is_lvalue(type_t* type) {
    return (bool)*((void**)type + TYPE_OFFSET_IS_LVALUE);
}

void type_set_lvalue(type_t* type, bool lvalue) {
    *((void**)type + TYPE_OFFSET_IS_LVALUE) = (void*)lvalue;
}

int type_indirections(type_t* type) {
    return (bool)*((void**)type + TYPE_OFFSET_INDIRECTION_COUNT);
}

void type_set_indirections(type_t* type, int count) {
    *((void**)type + TYPE_OFFSET_INDIRECTION_COUNT) = (void*)count;
}

int type_array_length(type_t* type) {
    return (int)*((void**)type + TYPE_OFFSET_ARRAY_SIZE);
}

void type_set_array_length(type_t* type, int array_length) {
    *((void**)type + TYPE_OFFSET_ARRAY_SIZE) = (void*)array_length;
}

size_t type_size(type_t* type) {
    size_t size:

    // TODO cci/0 forbids type_size() on lvalues, why? I don't remember

    // figure out the base size
    bool indirections = (type_indirections(type) > 0);
    if (indirections) {
        size = 4;
    }
    if (!indirections) {
        record_t* record = type_record(type);
        if (record) {
            size = record_size(record);
        }
        if (!record) {
            size = base_size(type_base(type));
        }
    }

    // if it's an array, multiply by the length
    size_t array_length = type_array_length(type);
    if (array_length == TYPE_ARRAY_NONE) {
        return size;
    }
    if (array_length == TYPE_ARRAY_INDETERMINATE) {
        fatal("Cannot sizeof() an array of indeterminate length.");
    }
    size = (size * array_length); // TODO overflow check
    return size;
}
#endif


// TODO new typedef table. should be one big table, but entries are name + tag, where tag is one of: nothing, union, struct, enum.









#define TYPE_LVALUE_MASK 0x80
#define TYPE_BASIC_MASK 0x60
#define TYPE_INDIRECTION_MASK 0x1F

static char** typedef_names;
static type_t* typedef_types;
static size_t typedef_count;

#define TYPEDEF_MAX 32

void typedef_init(void) {
    typedef_names = malloc(TYPEDEF_MAX * sizeof(char*));
    typedef_types = malloc(TYPEDEF_MAX * TYPE_T_SIZE);
}

void typedef_destroy(void) {
    for (size_t i = 0; i < typedef_count; ++i) {
        free(typedef_names[i]);
    }
    free(typedef_names);
    free(typedef_types);
}

void typedef_add(char* name, type_t type) {
    if (typedef_count == TYPEDEF_MAX) {
        fatal("Too many typedefs.");
    }
    typedef_names[typedef_count] = name;
    typedef_types[typedef_count] = type;
    ++typedef_count;
}

int typedef_find(const char* name, type_t* out_type) {
    for (size_t i = 0; i < typedef_count; ++i) {
        if (0 == strcmp(name, typedef_names[i])) {
            *out_type = typedef_types[i];
            return true;
        }
    }
    return false;
}

type_t type_make(int basic_type, int indirection) {
    if (indirection > TYPE_INDIRECTION_MASK) {
        fatal("Too many levels of indirection.");
    }
    return (type_t)(basic_type | indirection);
}

int type_size(type_t type) {
    if (type & TYPE_LVALUE_MASK) {
        fatal("Internal error: cannot return the size of an lvalue.");
    }

    // If there is no indirection, we can compare the basic types directly.
    if (type == TYPE_BASIC_VOID) {
        fatal("void type does not have a size.");
    }
    if (type == TYPE_BASIC_CHAR) {
        return 1;
    }

    // int is 4 and any indirection is 4 regardless of basic type.
    return 4;
}

int type_basic(type_t type) {
    return type & TYPE_BASIC_MASK;
}

int type_indirection(type_t type) {
    return type & TYPE_INDIRECTION_MASK;
}

type_t type_decrement_indirection(type_t type) {
    return ((type & TYPE_INDIRECTION_MASK) - 1) | (type & ~TYPE_INDIRECTION_MASK);
}
