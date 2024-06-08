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

#ifndef ONRAMP_LIBO_STRING_H_INCLUDED
#define ONRAMP_LIBO_STRING_H_INCLUDED

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "libo-table.h"

/**
 * A pre-hashed, reference-counted, length-delimited, null-terminated, interned
 * string.
 *
 * Allocated strings in the final stage Onramp tools are de-duplicated by this
 * string container. This minimizes memory usage and makes for fast string
 * comparisons and table lookups.
 */
typedef struct string_t {
    uint32_t refcount;
    table_entry_t entry;
    size_t length;
    char bytes[];
} string_t;

/**
 * Initializes the intern string table.
 */
void string_table_init(void);

/**
 * Destroys the intern string table.
 *
 * It must be empty! If any strings are leaked, it is a fatal error.
 */
void string_table_destroy(void);

/**
 * Returns a string containing the given null-terminated C string, creating it
 * if necessary and incrementing its reference count by one.
 *
 * You must dereference the string when done with it.
 *
 * If the string does not yet exist in the intern table, a new string will be
 * allocated and the given bytes will be copied into it.
 *
 * @see string_deref()
 */
string_t* string_intern_cstr(const char* s);

/**
 * Returns a string with the given byte contents, creating it if necessary and
 * incrementing its reference count by one.
 *
 * You must dereference the string when done with it.
 *
 * If the string does not yet exist in the intern table, a new string will be
 * allocated and the given bytes will be copied into it.
 *
 * @see string_deref()
 */
string_t* string_intern_bytes(const char* bytes, size_t length);

/**
 * Increments the reference count of the given string, returning it.
 */
string_t* string_ref(string_t* string);

/**
 * Decrements the reference count of the given string.
 *
 * If the reference count drops to zero, the string will be freed and removed
 * from the intern table.
 */
void string_deref(string_t* string);

/**
 * Returns true if the two given strings are equal.
 *
 * The strings are interned so we just compare pointers with operator ==. We
 * still use this wrapper for clarity.
 */
#ifndef __onramp_cpp_omc__
    #ifndef DEBUG
        #define string_equal(left, right) ((left) == (right))
    #endif
#endif
#ifndef string_equal
    static inline bool string_equal(const string_t* left, const string_t* right) {
        return left == right;
    }
#endif

/**
 * Returns true if the contents of the given string match the given
 * null-terminated C string.
 */
#ifndef __onramp_cpp_omc__
    #ifndef DEBUG
        #define string_equal_cstr(string, cstr) (0 == strcmp((string)->bytes, (cstr)))
    #endif
#endif
#ifndef string_equal_cstr
    static inline bool string_equal_cstr(const string_t* string, const char* cstr) {
        return 0 == strcmp(string->bytes, cstr);
    }
#endif

/**
 * Returns true if the contents of the given string match the given bytes.
 */
bool string_equal_bytes(const string_t* string, const char* bytes, size_t length);

/**
 * Prints the given string to the given file.
 */
void string_print(const string_t* string, FILE* file);

/**
 * Returns the given string as a null-terminated C string.
 *
 * This can be used for example with printf().
 */
#ifndef __onramp_cpp_omc__
    #ifndef DEBUG
        #define string_cstr(str) ((str)->bytes)
    #endif
#endif
#ifndef string_cstr
    static inline const char* string_cstr(const string_t* str) {
        return str->bytes;
    }
#endif

/**
 * Returns the FNV-1a hash of the given string.
 */
#ifndef __onramp_cpp_omc__
    #ifndef DEBUG
        #define string_hash(str) ((str)->entry.hash)
    #endif
#endif
#ifndef string_hash
    static inline uint32_t string_hash(const string_t* str) {
        return str->entry.hash;
    }
#endif

/**
 * Returns the length of the given string.
 */
#ifndef __onramp_cpp_omc__
    #ifndef DEBUG
        #define string_length(str) ((str)->length)
    #endif
#endif
#ifndef string_length
    static inline size_t string_length(const string_t* str) {
        return str->length;
    }
#endif

#endif
