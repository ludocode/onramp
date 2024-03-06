#include "type.h"

#include <stdlib.h>
#include <string.h>

#include "common.h"


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

/*
static type_t* type_new(void) {
    type_t* type = calloc(TYPE_FIELD_COUNT, sizeof(void*));
    if (!type) {
        fatal("Out of memory.");
    }
    return type;
}
*/

/*
type_t* type_new_basic(basic_type_t base, size_t indirection_count, size_t array_length) {
    type_t* type = type_new(indirection_count, array_length);
}

type_t* type_new_record(record_t* record, size_t indirection_count, size_t array_length) {
}
*/

/*
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

int type_indirections(const type_t* type) {
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
*/




// TODO above code is currently disabled, haven't fully migrated to new type yet


// TODO new typedef table. should be one big table, but entries are name + tag, where tag is one of: nothing, union, struct, enum.






#define TYPE_LVALUE_MASK 0x80
#define TYPE_BASIC_MASK 0x60
#define TYPE_INDIRECTION_MASK 0x1F

static char** typedef_names;
static type_t** typedef_types;
static size_t typedef_count;

// TODO huge number here to temporarily pass cci/0 tests, we're going to
// change this to a hashtable soon
#define TYPEDEF_MAX 512

type_t* type_new_blank(void) {
    return calloc(1, 1);
}

void typedef_init(void) {
    typedef_names = malloc(TYPEDEF_MAX * sizeof(char*));
    typedef_types = malloc(TYPEDEF_MAX * sizeof(type_t*));
}

void typedef_destroy(void) {
    size_t i = 0;
    while (i < typedef_count) {
        free(*(typedef_names + i));
        type_delete(*(typedef_types + i));
        i = (i + 1);
    }
    free(typedef_names);
    free(typedef_types);
}

void typedef_add(char* name, type_t* type) {
    if (typedef_count == TYPEDEF_MAX) {
        fatal("Too many typedefs.");
    }
    // TODO check for duplicates, allowed as long as the type matches
    *(typedef_names + typedef_count) = name;
    *(typedef_types + typedef_count) = type;
    typedef_count = (typedef_count + 1);
}

const type_t* typedef_find(const char* name) {
    size_t i = 0;
    while (i < typedef_count) {
        if (0 == strcmp(name, *(typedef_names + i))) {
            return *(typedef_types + i);
        }
        i = (i + 1);
    }
    return NULL;
}

type_t* type_new(int basic_type, int indirection) {
    type_t* type = type_new_blank();
    *type = (basic_type | indirection);
    return type;
}

type_t* type_clone(const type_t* other) {
    type_t* ret = type_new_blank();
    *ret = *other;
    return ret;
}

int type_size(const type_t* ptype) {
    type_t type = *ptype;
    if (type & TYPE_LVALUE_MASK) {
        // TODO ignore it, see parse_unary_expression(), going to fix this later
        type = (type & ~TYPE_LVALUE_MASK);
        //fatal("Internal error: cannot return the size of an lvalue.");
    }

    // If there is no indirection, we can compare the basic types directly.
    if (type == TYPE_BASIC_VOID) {
        return 1;
    }
    if (type == TYPE_BASIC_CHAR) {
        return 1;
    }

    // int is 4 and any indirection is 4 regardless of basic type.
    return 4;
}

base_t type_base(const type_t* type) {
    return *type & TYPE_BASIC_MASK;
}

void type_delete(type_t* type) {
    free(type);
}

void type_set_base(type_t* type, base_t base) {
    *type = ((*type & ~TYPE_BASIC_MASK) | base);
}

int type_indirections(const type_t* type) {
    return *type & TYPE_INDIRECTION_MASK;
}

void type_set_indirections(type_t* type, int count) {
    // TODO range
    *type = ((*type & ~TYPE_INDIRECTION_MASK) | count);
}

type_t* type_decrement_indirection(type_t* type) {
    *type = (((*type & TYPE_INDIRECTION_MASK) - 1) | (*type & ~TYPE_INDIRECTION_MASK));
    return type;
}

type_t* type_increment_indirection(type_t* type) {
    // TODO range
    *type = (((*type & TYPE_INDIRECTION_MASK) + 1) | (*type & ~TYPE_INDIRECTION_MASK));
    return type;
}

bool type_is_lvalue(const type_t* type) {
    return !!(*type & TYPE_LVALUE_MASK);
}

type_t* type_set_lvalue(type_t* type, bool lvalue) {
    if (lvalue) {
        *type = (*type | TYPE_LVALUE_MASK);
    }
    if (!lvalue) {
        *type = (*type & ~TYPE_LVALUE_MASK);
    }
    return type;
}

bool type_equal(const type_t* left, const type_t* right) {
    return *left == *right;
}
