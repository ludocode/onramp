/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Fraser Heavy Software
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

#ifndef TYPE_H_INCLUDED
#define TYPE_H_INCLUDED

#include <stdint.h>

#include "common.h"

struct token_t;
struct record_t;
struct enum_t;
struct scope_t;

/**
 * The kind of declarator element in a `type_t`.
 */
typedef enum declarator_t {
    DECLARATOR_POINTER,
    DECLARATOR_FUNCTION,
    DECLARATOR_ARRAY,          // an array of constant length
    DECLARATOR_VLA,            // an array of variable length
    DECLARATOR_INDETERMINATE,  // an array of indeterminate length
} declarator_t;

/**
 * The base typeof a `type_t` (that specified by the type specifiers and
 * qualifiers.)
 */
typedef enum base_t {

    // primitive base types
    BASE_VOID,
    BASE_BOOL,
    // Note that `char` and `signed char` are distinct types in C even though
    // `char` is signed. This differs from other types, where for example `int`
    // and `signed int` are the same type. This matters for _Generic among
    // other things.
    BASE_CHAR,
    BASE_SIGNED_CHAR,
    BASE_UNSIGNED_CHAR,
    BASE_SIGNED_SHORT,
    BASE_UNSIGNED_SHORT,
    BASE_SIGNED_INT,
    BASE_UNSIGNED_INT,
    BASE_SIGNED_LONG,
    BASE_UNSIGNED_LONG,
    BASE_SIGNED_LONG_LONG,
    BASE_UNSIGNED_LONG_LONG,
    BASE_FLOAT,
    BASE_DOUBLE,
    BASE_LONG_DOUBLE,

    // composite base types
    BASE_RECORD,
    BASE_ENUM,

} base_t;

size_t base_size(base_t);

/**
 * Converts a signed base to its corresponding unsigned base.
 */
base_t base_unsigned_of_signed(base_t);

/**
 * A `type_t` describes a type.
 *
 * One `type_t` struct encodes one element of a type. It can be a base type
 * (such as int, char, a struct, an enum) or it can be one element in a
 * declarator (such as a pointer, array or function declarator.)
 *
 * In the case of a declarator, it references other `type_t` elements which
 * describe the pointed-to type, the array element type, or the return and
 * argument types of functions.
 *
 * In the case of a struct or union, it references a `record_t` which describes
 * the members and their types, which are also `type_t` elements.
 *
 * A complete type is therefore a pointer to a `type_t`, which describes a
 * graph of `type_t` elements. `type_t` is reference-counted and immutable so
 * there is no possibility of cycles.
 */
typedef struct type_t {
    unsigned refcount;
    bool is_declarator : 1;

    bool is_const : 1;
    bool is_volatile : 1;

    union {
        struct {
            base_t base;
            union {
                struct record_t* record;
                struct enum_t* enum_;
            };
        };
        struct {
            declarator_t declarator;
            bool is_restrict : 1;
            bool is_variadic : 1;
            struct type_t* ref;     // pointed-to type or return type of function
            uint32_t count;         // array size or argument count
            struct type_t** args;   // function argument types
            struct token_t** names; // function argument names
            struct scope_t* scope;  // function argument scope
        };
    };
} type_t;

type_t* type_new_base(base_t element);
type_t* type_new_declarator(declarator_t declarator);
type_t* type_new_record(struct record_t* record);
type_t* type_new_enum(struct enum_t* enum_);
type_t* type_new_pointer(type_t* pointed_to_type, bool is_const, bool is_volatile, bool is_restrict);
type_t* type_new_array(type_t* element_type, uint32_t element_count);
type_t* type_new_function(type_t* return_type, type_t** arg_types,
        struct token_t** arg_names, uint32_t args_count, bool is_variadic);

// Reference-counting
static inline type_t* type_ref(type_t* type) {
    ++type->refcount;
    return type;
}

void type_deref(type_t* type);

/**
 * Returns a copy of the given type with the additional given qualifiers (or
 * the same type if they are all false.)
 */
type_t* type_qualify(type_t* type, bool is_const, bool is_volatile);

/**
 * Returns true if this is a pointer or array type.
 */
bool type_is_indirection(type_t* type);

/**
 * Prints a type in C syntax.
 */
void type_print(type_t* type);

/**
 * Prints a description of a type in words.
 */
void type_print_words(const type_t* type);


static inline bool type_is_base(type_t* type) {
    return !type->is_declarator;
}

static inline bool type_is_declarator(type_t* type) {
    return type->is_declarator;
}

/**
 * Returns true if the given type's base is that given.
 */
bool type_matches_base(type_t* type, base_t base);

bool type_is_arithmetic(type_t* type);

bool type_is_integer(type_t* type);

int type_integer_rank(type_t* type);

/**
 * The size of the given type, as in sizeof().
 */
size_t type_size(type_t* type);

size_t type_alignment(type_t* type);

/**
 * Returns true if the type is signed or unsigned int.
 */
bool type_is_int(type_t* type);

/**
 * Returns true if the type is signed or unsigned long long.
 */
bool type_is_long_long(type_t* type);

/**
 * Returns true if the type is a signed integer type (not a float.)
 */
bool type_is_signed_integer(type_t* type);

/**
 * Returns true if the given types match.
 */
bool type_equal(type_t* left, type_t* right);

/**
 * Returns true if the given types match, ignoring top-level qualifiers.
 */
bool type_equal_unqual(type_t* left, type_t* right);

/**
 * Returns true if the given types are compatible.
 */
bool type_compatible(type_t* left, type_t* right);

/**
 * Returns true if the given type is callable (i.e. it is a function or a
 * function pointer.
 */
bool type_is_callable(type_t* type);

bool type_is_array(type_t* type);

bool type_is_function(type_t* type);

/**
 * Returns true if values of this type are passed indirectly (on the stack);
 * false if they are passed in registers.
 *
 * For simplicity, the only types we pass in registers are base types that fit
 * (including enums) and pointers (including arrays, which decay to pointers
 * when passed.)
 *
 * All records are passed indirectly (even if they would otherwise fit) as well
 * as 64-bit base types (double, long long.) Nothing is ever passed in more
 * than one register.
 *
 * Note that void is considered to be passed directly, and we don't care what
 * the register contains.
 */
bool type_is_passed_indirectly(type_t* type);

/**
 * Decays arrays to pointers.
 *
 * Returns a strong reference, which may be the given type or a new type. You
 * must deref it when done.
 */
type_t* type_decay(type_t* type);

/**
 * Returns the pointed-to type for a pointer or array.
 */
type_t* type_pointed_to(type_t* type);

#endif
