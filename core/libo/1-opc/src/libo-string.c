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

#include "libo-string.h"

#include <stdlib.h>
#include <string.h>

#include "libo-error.h"
#include "libo-util.h"

static table_t string_table;

#ifndef __onramp_cpp_omc__
    #define string_from_entry(entry) (containerof(entry, string_t, entry))
#endif
#ifndef string_from_entry
    static string_t* string_from_entry(table_entry_t* entry) {
        return (string_t*)((char*)entry - (unsigned)&((string_t*)0)->entry);
    }
#endif

void strings_init(void) {
    table_init(&string_table);
}

void strings_destroy(void) {
    if (table_count(&string_table) != 0) {
        // We don't log a fatal error under ASAN because it will report the
        // leak on its own.
        #ifndef __SANITIZE_ADDRESS__
        fatal("Internal error: a string was leaked.");
        #endif
    }
    table_destroy(&string_table);
}

string_t* string_intern_bytes(const char* bytes, size_t length) {

    // look for an existing string in the table
    uint32_t hash = fnv1a_bytes(bytes, length);
    table_entry_t* entry = table_bucket(&string_table, hash);
    while (entry) {
        if (hash == table_entry_hash(entry)) {
            string_t* candidate = string_from_entry(entry);
            if (string_equal_bytes(candidate, bytes, length)) {
                // existing string found
                return string_ref(candidate);
            }
        }
        entry = table_entry_next(entry);
    }

    // string not found. allocate a new one
    string_t* string = malloc(sizeof(string_t) + length + 1);
    string->refcount = 1;
    string->length = length;
    memcpy(string->bytes, bytes, length);
    string->bytes[length] = 0;

    // insert it into the intern table
    table_put(&string_table, &string->entry, hash);

    return string;
}

string_t* string_intern_cstr(const char* s) {
    return string_intern_bytes(s, strlen(s));
}

string_t* string_ref(string_t* string) {
    ++string->refcount;
    return string;
}

void string_deref(string_t* string) {
    if (--string->refcount != 0) {
        return;
    }

    // the string is no longer in use. remove it from the intern table
    // and free it.
    table_remove(&string_table, &string->entry);
    free(string);
}

bool string_equal_bytes(const string_t* string, const char* bytes, size_t length) {
    if (length != string->length) {
        return false;
    }
    return 0 == memcmp(string->bytes, bytes, length);
}

void string_print(const string_t* string, FILE* file) {
    fwrite(string->bytes, 1, string->length, file);
}
