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

#include "types.h"

#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "record.h"

#define NAMESPACE_TYPEDEF 1
#define NAMESPACE_TAG 2

/*
 * Types are store in a simple hashtable with open addressing. A bucket is
 * empty if its name is NULL.
 *
 * The name and namespace together form a key. The same name can be used in
 * different namespaces (because, for example, a typedef of `foo` is different
 * from `struct foo`.)
 */
static char** types_names;
static int* types_namespaces; // typedef, struct or union
static void** types_objects; // type_t* or record_t*
static size_t types_buckets;
static size_t types_count;

static size_t types_anonymous_records_capacity;
static size_t types_anonymous_records_count;
static record_t** types_anonymous_records;

void types_init(void) {
    types_buckets = 512;
    types_names = calloc(types_buckets, sizeof(char*));
    types_namespaces = malloc(types_buckets * sizeof(int));
    types_objects = malloc(types_buckets * sizeof(void*));

    types_anonymous_records_capacity = 256;
    types_anonymous_records = malloc(types_anonymous_records_capacity * sizeof(record_t*));

    if (types_names == NULL) {fatal("Out of memory.");}
    if (types_namespaces == NULL) {fatal("Out of memory.");}
    if (types_objects == NULL) {fatal("Out of memory.");}
    if (types_anonymous_records == NULL) {fatal("Out of memory.");}

    // We store __builtin_va_list as a typedef of int*. This means it's
    // implicitly convertible with int*, can be copied by value, etc. That's
    // fine; we don't need to diagnose improper use.
    type_t* list = type_new_base(BASE_SIGNED_INT);
    list = type_increment_pointers(list);
    types_add_typedef(strdup_checked("__builtin_va_list"), list);
}

void types_destroy(void) {

    // free hashtable contents
    size_t i = 0;
    while (i < types_buckets) {
        char* name = *(types_names + i);
        if (name != NULL) {
            free(name);
            int namespace = *(types_namespaces + i);
            if (namespace == NAMESPACE_TYPEDEF) {
                type_delete(*(types_objects + i));
            }
            if (namespace == NAMESPACE_TAG) {
                record_delete(*(types_objects + i));
            }
        }
        i = (i + 1);
    }

    // free anonymous records
    i = 0;
    while (i < types_anonymous_records_count) {
        record_delete(*(types_anonymous_records + i));
        i = (i + 1);
    }
}

void types_add_anonymous_record(record_t* record) {
    if (types_anonymous_records_count == types_anonymous_records_capacity) {
        // TODO make the anonymous types list growable.
        fatal("Too many anonymous structs or unions.");
    }
    *(types_anonymous_records + types_anonymous_records_count) = record;
    types_anonymous_records_count = (types_anonymous_records_count + 1);
}

/**
 * Finds the bucket for a type or record with the given name, or the empty
 * bucket where it should be inserted if it doesn't exist.
 */
int types_find_bucket(const char* name, int namespace) {
    int hash = (fnv1a_cstr(name) + (namespace * 31));
    int mask = (types_buckets - 1);
    int index = (hash & mask);
    while (1) {
        char* bucket_name = *(types_names + index);
        int bucket_namespace = *(types_namespaces + index);
        if (bucket_name != NULL) {
            if (namespace == bucket_namespace) {
                if (0 == strcmp(name, bucket_name)) {
                    return index;
                }
            }
        }
        if (bucket_name == NULL) {
            return index;
        }
        index = ((index + 1) & mask);
    }
}

type_t* types_add_typedef(char* name, type_t* type) {
    int index = types_find_bucket(name, NAMESPACE_TYPEDEF);
    if (*(types_names + index) != NULL) {
        // The typedef already exists. Return the existing type.
        free(name);
        type_delete(type);
        return *(types_objects + index);
    }

    // The typedef does not exist. Add it.

    if (types_count == (types_buckets >> 1)) {
        // TODO make the hashtable growable.
        fatal("Too many typedefs, structs or unions.");
    }
    types_count = (types_count + 1);

    *(types_names + index) = name;
    *(types_namespaces + index) = NAMESPACE_TYPEDEF;
    *(types_objects + index) = type;
    return type;
}

void types_add_record(record_t* record) {
    const char* name = record_name(record);
    int index = types_find_bucket(name, NAMESPACE_TAG);
    if (*(types_names + index) != NULL) {
        // If this happens there's a bug; the parser should be checking for
        // pre-existing struct/union declarations first.
        fatal("Internal error: record already exists.");
    }

    // Add the record
    *(types_names + index) = strdup_checked(name);
    *(types_namespaces + index) = NAMESPACE_TAG;
    *(types_objects + index) = record;
}

void* types_find_object(const char* name, int namespace) {
    int index = types_find_bucket(name, namespace);
    if (*(types_names + index) != NULL) {
        return *(types_objects + index);
    }
    return NULL;
}

const type_t* types_find_typedef(const char* name) {
    return types_find_object(name, NAMESPACE_TYPEDEF);
}

record_t* types_find_record(const char* name) {
    return types_find_object(name, NAMESPACE_TAG);
}

