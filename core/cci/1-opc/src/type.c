#include "type.h"

#include <stdlib.h>
#include <string.h>

#include "common.h"
//#include "record.h"



/*
 * Base types
 */

static size_t base_size(base_t base) {
    if (base == BASE_VOID) {
        // For some reason sizeof(void) is 1.
        return 1;
    }
    if (base == BASE_UNSIGNED_CHAR) {return 1;}
    if (base == BASE_UNSIGNED_SHORT) {return 2;}
    if (base == BASE_UNSIGNED_INT) {return 4;}
    if (base == BASE_UNSIGNED_LONG_LONG) {return 8;}
    if (base == BASE_SIGNED_CHAR) {return 1;}
    if (base == BASE_SIGNED_SHORT) {return 2;}
    if (base == BASE_SIGNED_INT) {return 4;}
    if (base == BASE_SIGNED_LONG_LONG) {return 8;}

    if (base == BASE_RECORD) {
        // it's a record. should be calling record_size(), not base_size()
        fatal("Internal error: cannot base_size(BASE_RECORD)");
    }

    fatal("Internal error: invalid base type");
}

static const char* base_to_string(base_t base) {
    if (base == BASE_RECORD) {return "record";}
    if (base == BASE_VOID) {return "void";}
    if (base == BASE_UNSIGNED_CHAR) {return "unsigned char";}
    if (base == BASE_UNSIGNED_SHORT) {return "unsigned short";}
    if (base == BASE_UNSIGNED_INT) {return "unsigned int";}
    if (base == BASE_UNSIGNED_LONG_LONG) {return "unsigned long long";}
    if (base == BASE_SIGNED_CHAR) {return "signed char";}
    if (base == BASE_SIGNED_SHORT) {return "signed short";}
    if (base == BASE_SIGNED_INT) {return "signed int";}
    if (base == BASE_SIGNED_LONG_LONG) {return "signed long long";}
    fatal("Invalid base type");
}

/*
static bool base_is_signed(base_t base) {
    if (base == BASE_SIGNED_CHAR) {return true;}
    if (base == BASE_SIGNED_SHORT) {return true;}
    if (base == BASE_SIGNED_INT) {return true;}
    if (base == BASE_SIGNED_LONG_LONG) {return true;}
    return false;
}
*/



/*
 * Types
 */

/*
 * type_t looks something like this:
 *
 * typedef struct {
 *     basic_type_t base;
 *     record_t* record;
 *     int pointers;
 *     int array_length;
 *     bool is_lvalue;
 * } type_t;
 *
 * We make each field a void* for simplicity.
 */

#define TYPE_OFFSET_BASE 0
#define TYPE_OFFSET_RECORD 1
#define TYPE_OFFSET_POINTERS 2
#define TYPE_OFFSET_ARRAY_LENGTH 3
#define TYPE_OFFSET_IS_LVALUE 4
#define TYPE_FIELD_COUNT 5

void type_delete(type_t* type) {
    free(type);
}

static type_t* type_new(void) {
    type_t* type = calloc(TYPE_FIELD_COUNT, sizeof(void*));
    if (!type) {
        fatal("Out of memory.");
    }
    type_set_array_length(type, TYPE_ARRAY_NONE);
    return type;
}

type_t* type_new_base(base_t base) {
    type_t* type = type_new();
    type_set_base(type, base);
    return type;
}

type_t* type_clone(const type_t* other) {
    type_t* type = memdup(other, TYPE_FIELD_COUNT * sizeof(void*));
    if (!type) {
        fatal("Out of memory.");
    }
    return type;
}

base_t type_base(const type_t* type) {
    return *(base_t*)((void**)type + TYPE_OFFSET_BASE);
}

void type_set_base(type_t* type, base_t base) {
    *(base_t*)((void**)type + TYPE_OFFSET_BASE) = base;
}

record_t* type_record(const type_t* type) {
    return *(record_t**)((void**)type + TYPE_OFFSET_RECORD);
}

void type_set_record(type_t* type, record_t* record) {
    *(record_t**)((void**)type + TYPE_OFFSET_RECORD) = record;
}

bool type_is_lvalue(const type_t* type) {
    return *(bool*)((void**)type + TYPE_OFFSET_IS_LVALUE);
}

type_t* type_set_lvalue(type_t* type, bool lvalue) {
    *(bool*)((void**)type + TYPE_OFFSET_IS_LVALUE) = lvalue;
    return type;
}

int type_indirections(const type_t* type) {
    int pointers = type_pointers(type);
    if (type_array_length(type) != TYPE_ARRAY_NONE) {
        pointers = (pointers + 1);
    }
    return pointers;
}

int type_pointers(const type_t* type) {
    return *(int*)((void**)type + TYPE_OFFSET_POINTERS);
}

void type_set_pointers(type_t* type, int count) {
    *(int*)((void**)type + TYPE_OFFSET_POINTERS) = count;
}

bool type_is_array(const type_t* type) {
    return type_array_length(type) != TYPE_ARRAY_NONE;
}

int type_array_length(const type_t* type) {
    return *(int*)((void**)type + TYPE_OFFSET_ARRAY_LENGTH);
}

void type_set_array_length(type_t* type, int array_length) {
    *(int*)((void**)type + TYPE_OFFSET_ARRAY_LENGTH) = array_length;
}

bool type_is_record(const type_t* type) {
    return type_base(type) == BASE_RECORD;
}

size_t type_size(const type_t* type) {
    size_t size;

    // TODO cci/0 forbids type_size() on lvalues, why? I don't remember

    // figure out the element size
    int pointers = type_pointers(type);
    if (pointers > 0) {
        size = 4;
    }
    if (pointers == 0) {
        record_t* record = type_record(type);
        if (record) {
            fatal("record size not yet implemented");
            //size = record_size(record);
        }
        if (!record) {
            size = base_size(type_base(type));
        }
    }

    // if it's an array, multiply by the length
    int array_length = type_array_length(type);
    if (array_length == TYPE_ARRAY_NONE) {
        return size;
    }
    if (array_length == TYPE_ARRAY_INDETERMINATE) {
        fatal("Cannot sizeof() an array of indeterminate length.");
    }
    size = (size * array_length); // TODO overflow check
    return size;
}

type_t* type_decrement_indirection(type_t* type) {
    if (type_is_array(type)) {
        type_set_array_length(type, TYPE_ARRAY_NONE);
        return type;
    }
    int pointers = type_pointers(type);
    if (pointers == 0) {
        fatal("Internal error: cannot decrement indirections of scalar type");
    }
    type_set_pointers(type, pointers - 1);
    return type;
}

type_t* type_increment_pointers(type_t* type) {
    type_set_pointers(type, type_pointers(type) + 1);
    return type;
}

type_t* type_increment_indirection(type_t* type) {
    if (type_is_array(type)) {
        // TODO there are some cases where taking the address decays without
        // incrementing indirection count. we need to differentiate between these
        // cases. For now we assume we're taking the address of an array, which
        // does *not* increment indirections.
        type_set_array_length(type, TYPE_ARRAY_NONE);
    }
    return type_increment_pointers(type);
}

bool type_equal(const type_t* left, const type_t* right) {
    if (type_base(left) != type_base(right)) {
        return false;
    }
    if (type_record(left) != type_record(right)) {
        return false;
    }
    if (type_is_lvalue(left) != type_is_lvalue(right)) {
        return false;
    }
    if (type_pointers(left) != type_pointers(right)) {
        return false;
    }
    if (type_array_length(left) != type_array_length(right)) {
        return false;
    }
    return true;
}



/*
 * Typedefs
 */

// TODO new typedef table. should be one big table, but entries are name + tag, where tag is one of: nothing, union, struct, enum.

static char** typedef_names;
static type_t** typedef_types;
static size_t typedef_count;

// TODO huge number here to temporarily pass cci/0 tests, we're going to
// change this to a hashtable soon
#define TYPEDEF_MAX 512

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

bool type_is_base(const type_t* type, base_t base) {
    assert(base != BASE_RECORD);
    if (type_base(type) != base) {
        return false;
    }
    if (type_indirections(type) != 0) {
        return false;
    }
    if (type_is_lvalue(type)) {
        return false;
    }
    return true;
}

/*
type_t* type_decay_array(type_t* type) {
    if (type_is_array(type)) {
        type_set_array_length(type, TYPE_ARRAY_NONE);
        type_increment_pointers(type);
    }
    return type;
}
*/

void type_print(const type_t* type) {

    // print base
    base_t base = type_base(type);
    if (base == BASE_RECORD) {
        fatal("TODO type_print() record");
    }
    if (base != BASE_RECORD) {
        fputs(base_to_string(base), stdout);
    }

    // count pointers
    int pointers = type_pointers(type);

    // print pointers
    int i = 0;
    while (i < pointers) {
        putchar('*');
        i = (i + 1);
    }

    // print array_length
    int array_length = type_array_length(type);
    if (array_length != TYPE_ARRAY_NONE) {
        putchar('[');
        if (array_length != TYPE_ARRAY_INDETERMINATE) {
            putd(array_length);
        }
        putchar(']');
    }
}
