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

#ifndef TYPE_H_INCLUDED
#define TYPE_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>

#include "common.h"

/**
 * A type in opC is described by:
 *
 * - a base primitive or record type
 * - a pointer count
 * - an optional array size, which can be indeterminate
 * - an l-value flag (describing its state of compilation)
 */

/**
 * A base type.
 *
 * `char`, `signed char` and `unsigned char` are technically supposed to be
 * three distinct types in C. `int` and `long` are also supposed to be
 * distinct. This matters for `_Generic` among other things, but we don't
 * support that in opC so we don't bother differentating between these.
 *
 * In opC, `char` and `signed char` are the same thing, and we don't have a
 * `long` type; `long` is treated as `int` after parsing.
 */
typedef int base_t;

#define BASE_INVALID 0
#define BASE_RECORD 1
#define BASE_VOID 2
#define BASE_UNSIGNED_CHAR 3
#define BASE_UNSIGNED_SHORT 4
#define BASE_UNSIGNED_INT 5
#define BASE_SIGNED_CHAR 7
#define BASE_SIGNED_SHORT 8
#define BASE_SIGNED_INT 9

/**
 * Note: We treat an array of size zero as distinct from an array of
 * indeterminate size from a parsing perspective. e.g. This is invalid:
 *
 *     char buffer[0] = "Hello";
 *
 * However, this is valid:
 *
 *     char buffer[] = "Hello";
 *
 * It is parsed as indeterminate size and is changed to the proper size after
 * parsing, this way e.g. sizeof() works.
 *
 * However, we do allow, as an extension, a zero-size array as the last element
 * of a struct. This is treated as an alias for a flexible array member, but we
 * actually store it the other way around: the flexible array member is
 * converted to a size zero array after parsing, that way we don't need to do
 * anything special to make sizeof() work on a struct with a flexible array
 * member.
 */
#define TYPE_ARRAY_NONE -1
#define TYPE_ARRAY_INDETERMINATE -2

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

type_t* type_new_record(const record_t* record);

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

const record_t* type_record(const type_t* type);

void type_set_record(type_t* type, const record_t* record);

/**
 * Returns the indirection count of the given type, which includes pointers and
 * arrays.
 *
 * For example `int**` has two indirections and `int**[5]` has three.
 *
 * This does not include the l-value flag.
 */
int type_indirections(const type_t* type);

/**
 * Returns the pointer count, not including arrays.
 *
 * For example `int**` and `int**[5]` both have two pointers.
 */
int type_pointers(const type_t* type);

void type_set_pointers(type_t* type, int count);

/**
 * Decrements the indirection count of the given type, keeping the rest the
 * same.
 */
type_t* type_decrement_indirection(type_t* type);

type_t* type_increment_pointers(type_t* type);

bool type_is_lvalue(const type_t* type);

type_t* type_set_lvalue(type_t* type, bool lvalue);

bool type_is_array(const type_t* type);

/**
 * Returns the array length, or TYPE_ARRAY_NONE if it is not an array, or
 * TYPE_ARRAY_INDETERMINATE if it is an array of indeterminate length.
 */
int type_array_length(const type_t* type);

void type_set_array_length(type_t* type, int array_length);

/**
 * If the type has an array, it is converted to a pointer.
 */
type_t* type_decay_array(type_t* type);

bool type_equal(const type_t* left, const type_t* right);

/**
 * Returns true if a type exactly matches the given base with no pointers,
 * array, record, etc.
 */
bool type_is_base(const type_t* type, base_t base);

void type_print(const type_t* type);

bool type_is_signed(const type_t* type);

bool type_is_unsigned(const type_t* type);

bool type_is_integer(const type_t* type);

bool type_is_compatible(const type_t* left, const type_t* right);

bool type_is_void_pointer(const type_t* type);

#endif
