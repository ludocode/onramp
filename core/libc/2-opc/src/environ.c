/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024-2025 Fraser Heavy Software
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

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <__onramp/__pit.h>

#include "internal.h"


/**
 * environ and the pointers contained in it originally point into memory
 * provided by the VM. Additionally, the user can directly assign their own
 * pointer to environ, and can provide their own pointers that are not copied
 * in putenv().
 *
 * In these cases, when the environment is modified by setenv() or putenv(),
 * we discard such pointers and allocate new ones. However if we allocated the
 * relevant pointer ourselves, we can grow, modify and free it.
 *
 * We keep track of allocated pointers in a set, stored as an ad hoc open
 * addressed hash table using tombstones for deletion. Whenever we want to
 * modify or free a pointer, we check if we allocated it via putenv() or
 * setenv() first. We never leak pointers; this way we can perform accurate
 * leak checking on shutdown even if a program used setenv()/putenv() or
 * assigned to environ.
 *
 *
 * TODO this is a simple pointer set which has nothing to do with environ. it
 * will probably be useful elsewhere in the libc, and even if it's not it's
 * better to pull it out of this file anyway.
 *
 */
static void** ptrs;
static size_t ptrs_bits; // buckets == 1 << bits
static size_t ptrs_count;
static size_t ptrs_tombstones;
#define TOMBSTONE ((void*)(-1))

static inline uint32_t ptrs_hash(void* ptr) {
    // simple Knuth hash on the address
    return (uint32_t)(((uintptr_t)ptr * 2654435761u) >> (sizeof(uintptr_t) * CHAR_BIT - ptrs_bits));
}

// Returns false if out of memory.
static bool ptrs_resize(void) {

    // allocate a new table large enough to fit
    // (we need at least 4x the count because we only allow it to fill to half)
    size_t new_bits = 4u;
    while ((1u << new_bits) < (4u * ptrs_count))
        ++new_bits;
    size_t new_buckets = 1u << new_bits;
    size_t new_mask = new_buckets - 1u;
    void** new_ptrs = calloc(new_buckets, sizeof(void*));
    if (new_ptrs == NULL) {
        return false;
    }

    // replace the old table
    void** old_ptrs = ptrs;
    size_t old_bits = ptrs_bits;
    ptrs = new_ptrs;
    ptrs_bits = new_bits;
    ptrs_tombstones = 0;

    // move pointers from the old table to the new one
    if (old_ptrs) {
        size_t old_buckets = 1u << old_bits;
        for (size_t i = 0; i < old_buckets; ++i) {
            void* p = old_ptrs[i];
            if (!p || p == TOMBSTONE)
                continue;
            size_t bucket = ptrs_hash(p) & new_mask;
            while (new_ptrs[bucket])
                bucket = (bucket + 1u) & new_mask;
            new_ptrs[bucket] = p;
        }
        free(old_ptrs);
    }

    return true;
}

// Returns false if out of memory.
static bool ptrs_add(void* p) {

    // we don't allow the table to get more than half full.
    if (2u * (ptrs_count + ptrs_tombstones + 1) >= (1u << ptrs_bits)) {
        if (!ptrs_resize()) {
            return false;
        }
    }

    size_t buckets = 1u << ptrs_bits;
    size_t mask = buckets - 1u;
    size_t bucket = ptrs_hash(p) & mask;
    for (;;) {
        void* v = ptrs[bucket];
        if (v == NULL || v == TOMBSTONE) {
            ptrs[bucket] = p;
            ++ptrs_count;
            if (v == TOMBSTONE) --ptrs_tombstones;
            return true;
        }
        bucket = (bucket + 1) & mask;
    }

    __builtin_unreachable();
}

// Returns true if it was removed, false if it didn't exist.
static bool ptrs_remove(void* p) {
    if (ptrs_bits == 0)
        return false;

    size_t buckets = 1 << ptrs_bits;
    size_t mask = buckets - 1;

    size_t bucket = ptrs_hash(p) & mask;
    for (;;) {
        void* v = ptrs[bucket];
        if (v == p) {
            ptrs[bucket] = TOMBSTONE;
            --ptrs_count;
            ++ptrs_tombstones;
            return true;
        }
        if (v == NULL)
            return false;
        bucket = (bucket + 1) & mask;
    }

    __builtin_unreachable();
}



/*
 * Environment implementation follows.
 */

static char* empty_environment[1];

/*
 * The environment pointer.
 *
 * The user is allowed to change this so we can't cache anything. We have to
 * parse it from scratch any time an environment function is called.
 */
char** environ;

void __environ_setup(void) {
    environ = (char**)__process_info_table[__ONRAMP_PIT_ENVIRON];

    // The VM is allowed to leave the environment NULL. We make sure it is at
    // least an empty array.
    if (environ == 0) {
        environ = empty_environment;
    }
}

void __environ_teardown(void) {
    // TODO if checking for leaks, free all pointers.
    // TODO this is not even called yet
}

char* getenv(const char* name) {
    size_t namelen = strlen(name);

    for (char** p = environ; *p; ++p) {
        char* item = *p;
        if (0 == strncmp(item, name, namelen)) {
            if (item[namelen] == '=') {
                return item + namelen + 1;
            }
        }
    }

    // not found
    return NULL;
}

static int environ_append(size_t environ_count, char* str) {

    // If we previously allocated the environ, we can realloc it.
    bool removed = ptrs_remove(environ);
    char** new_environ = realloc(removed ? environ : NULL, (environ_count + 2) * sizeof(char*));
    if (!new_environ) {
        errno = ENOMEM;
        return -1;
    }
    if (!ptrs_add(new_environ)) {
        if (removed) {
            // TODO we can't actually undo the realloc.  There's probably no
            // point in trying to handle this correctly.
            __fatal("Out of memory.");
        }
        free(new_environ);
        errno = ENOMEM;
        return -1;
    }

    // Set up the new environment
    if (!removed) {
        memcpy(new_environ, environ, environ_count * sizeof(char*));
    }
    new_environ[environ_count] = str;
    new_environ[environ_count + 1] = NULL;
    environ = new_environ;
    return 0;

}

int putenv(char* str) {
    char* equal = strchr(str, '=');
    if (equal == NULL) {
        // TODO this is a GNU extension. Should we require _GNU_SOURCE?
        return unsetenv(equal);
    }
    size_t name_eq_len = equal - str + 1; // including '='

    // Replace it if possible
    char** p = environ;
    for (; *p; ++p) {
        char* item = *p;
        if (0 == strncmp(item, str, name_eq_len)) {
            // Matching name found. Replace it without copying.
            *p = str;
            return 0;
        }
    }

    // Otherwise append.
    return environ_append(p - environ, str);

}

// TODO what to do on setenv() when value is NULL? z/OS ignores the flag and
// removes:
//     https://www.ibm.com/docs/en/zos/2.4.0?topic=functions-setenv-add-delete-change-environment-variables
// POSIX and glibc don't seem to specify:
//     https://pubs.opengroup.org/onlinepubs/9699919799/functions/setenv.html
//     https://man.archlinux.org/man/core/man-pages/setenv.3.en

static char* setenv_alloc(const char* name, size_t namelen, const char* value) {
    size_t valuelen = strlen(value);
    char* str = malloc(namelen + valuelen + 2);
    if (str == NULL) {
        return NULL;
    }
    if (!ptrs_add(str)) {
        free(str);
        return NULL;
    }
    memcpy(str, name, namelen);
    str[namelen] = '=';
    memcpy(str + namelen + 1, value, valuelen);
    str[namelen + 1 + valuelen] = 0;
    return str;
}

int setenv(const char* name, const char* value, int overwrite) {

    // Some required checks
    if (name == NULL) {
        errno = EINVAL;
        return -1;
    }
    size_t namelen = strlen(name);
    // TODO glibc man page says setenv() raises EINVAL if it contains '='. Does
    // that mean we have to check it?
    if (namelen == 0 || memchr(name, '=', namelen)) {
        errno = EINVAL;
        return -1;
    }

    // Replace an existing entry if possible
    char** p = environ;
    for (; *p; ++p) {
        char* item = *p;
        if (0 == strncmp(item, name, namelen) && item[namelen] == '=') {

            // Found. If not overwriting, skip.
            if (!overwrite) {
                return 0;
            }

            // Replace it (with a copy)
            char* str = setenv_alloc(name, namelen, value);
            if (str == NULL) {
                errno = ENOMEM;
                return -1;
            }
            if (ptrs_remove(*p)) {
                free(*p);
            }
            *p = str;
            return 0;
        }
    }

    // Otherwise append (a copy)
    char* str = setenv_alloc(name, namelen, value);
    if (str == NULL) {
        errno = ENOMEM;
        return -1;
    }
    int append = environ_append(p - environ, str);
    if (append != 0) {
        free(str);
    }
    return append;
}

int unsetenv(const char* name) {

    // Same checks as setenv() here.
    if (name == NULL) {
        errno = EINVAL;
        return -1;
    }
    size_t namelen = strlen(name);
    // TODO glibc man page says unsetenv() raises EINVAL if it contains '='.
    // Does that mean we have to check it?
    if (namelen == 0 || memchr(name, '=', namelen)) {
        errno = EINVAL;
        return -1;
    }

    // Find the existing entry
    char** p = environ;
    for (;; ++p) {
        char* item = *p;
        if (item == NULL) {
            // not found.
            return 0;
        }
        if (0 == strncmp(item, name, namelen) && item[namelen] == '=') {
            break;
        }
    }

    // Free it if we allocated it
    if (ptrs_remove(*p)) {
        free(*p);
    }

    // If this is the last entry we can just null it out.
    if (p[1] == NULL) {
        *p = NULL;
        return 0;
    }

    // Otherwise replace it with the last entry. (Note that this changes the
    // order of entries. If this is wrong, we'll have to memmove().)
    char** last = p;
    do ++last; while (*last);
    --last;
    *p = *last;
    *last = NULL;

    // TODO if we allocated environ we could realloc to shrink it. There's
    // probably no point so for now we don't bother.
    return 0;
}

int clearenv(void) {

    // clear the environment
    environ = empty_environment;

    // free all allocated pointers
    size_t buckets = 1 << ptrs_bits;
    for (size_t i = 0; i < buckets; ++i) {
        char* p = ptrs[i];
        if (p && p != TOMBSTONE) {
            free(p);
        }
    }

    // clear the table
    free(ptrs);
    ptrs = 0;
    ptrs_bits = 0;
    ptrs_count = 0;
    ptrs_tombstones = 0;

    return 0;
}
