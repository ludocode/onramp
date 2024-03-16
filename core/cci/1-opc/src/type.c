/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2024 Fraser Heavy Software
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "type.h"

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "record.h"



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
    if (base == BASE_SIGNED_CHAR) {return 1;}
    if (base == BASE_SIGNED_SHORT) {return 2;}
    if (base == BASE_SIGNED_INT) {return 4;}

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
    if (base == BASE_SIGNED_CHAR) {return "signed char";}
    if (base == BASE_SIGNED_SHORT) {return "signed short";}
    if (base == BASE_SIGNED_INT) {return "signed int";}
    fatal("Invalid base type");
}

/*
static bool base_is_signed(base_t base) {
    if (base == BASE_SIGNED_CHAR) {return true;}
    if (base == BASE_SIGNED_SHORT) {return true;}
    if (base == BASE_SIGNED_INT) {return true;}
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
 *     const record_t* record;
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

type_t* type_new_record(const record_t* record) {
    type_t* type = type_new();
    type_set_base(type, BASE_RECORD);
    type_set_record(type, record);
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

const record_t* type_record(const type_t* type) {
    return *(const record_t**)((void**)type + TYPE_OFFSET_RECORD);
}

void type_set_record(type_t* type, const record_t* record) {
    *(const record_t**)((void**)type + TYPE_OFFSET_RECORD) = record;
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

size_t type_size(const type_t* type) {
    size_t size;

    // TODO cci/0 forbids type_size() on lvalues, why? I don't remember

    // figure out the element size
    int pointers = type_pointers(type);
    if (pointers > 0) {
        size = 4;
    }
    if (pointers == 0) {
        const record_t* record = type_record(type);
        if (record) {
            size = record_size(record);
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

type_t* type_decay_array(type_t* type) {
    if (type_is_array(type)) {
        type_set_array_length(type, TYPE_ARRAY_NONE);
        type_increment_pointers(type);
    }
    return type;
}

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

    // print l-value
    if (type_is_lvalue(type)) {
        fputs(" {lv}", stdout);
    }
}

bool type_is_unsigned(const type_t* type) {
    if (type_indirections(type) != 0) {
        return false;
    }
    base_t base = type_base(type);
    if (base == BASE_UNSIGNED_CHAR) {return true;}
    if (base == BASE_UNSIGNED_SHORT) {return true;}
    if (base == BASE_UNSIGNED_INT) {return true;}
    return false;
}

bool type_is_signed(const type_t* type) {
    if (type_indirections(type) != 0) {
        return false;
    }
    base_t base = type_base(type);
    if (base == BASE_SIGNED_CHAR) {return true;}
    if (base == BASE_SIGNED_SHORT) {return true;}
    if (base == BASE_SIGNED_INT) {return true;}
    return false;
}

bool type_is_integer(const type_t* type) {
    if (type_indirections(type) != 0) {
        return false;
    }
    base_t base = type_base(type);
    if (base == BASE_UNSIGNED_CHAR) {return true;}
    if (base == BASE_UNSIGNED_SHORT) {return true;}
    if (base == BASE_UNSIGNED_INT) {return true;}
    if (base == BASE_SIGNED_CHAR) {return true;}
    if (base == BASE_SIGNED_SHORT) {return true;}
    if (base == BASE_SIGNED_INT) {return true;}
    return false;
}

bool type_is_void_pointer(const type_t* type) {
    if (type_is_array(type)) {
        return false;
    }
    if (type_pointers(type) != 1) {
        return false;
    }
    return type_base(type) == BASE_VOID;
}

bool type_is_compatible(const type_t* left, const type_t* right) {

    // Matching types compare equal.
    if (type_equal(left, right)) {
        return true;
    }

    // Matching base types with the same indirection count compare equal. (We
    // decay arrays to pointers in a comparison.)
    if (type_indirections(left) == type_indirections(right)) {
        if (type_base(left) == type_base(right)) {
            if (type_base(left) == BASE_RECORD) {
                if (type_record(left) != type_record(right)) {
                    return false;
                }
            }
            return true;
        }
    }

    // Check if both are pointers
    if (type_indirections(left) > 0) {
        if (type_indirections(right) > 0) {

            // Void pointers can be compared to pointers of any other type.
            if (type_is_void_pointer(left)) {
                return true;
            }
            if (type_is_void_pointer(right)) {
                return true;
            }

            // Otherwise pointers must exactly match.
            return false;
        }
    }

    // Otherwise at least one side is an integer. We need to allow pointers to
    // be compared to a literal 0 and we don't have a great way of tracking
    // this. For now we just allow all integers to be compared with anything.
    return true;
}
