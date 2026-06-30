/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Fraser Heavy Software
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

#ifndef ONRAMP_LIBO_OTABLE_H_INCLUDED
#define ONRAMP_LIBO_OTABLE_H_INCLUDED

/**
 * @file
 *
 * A simple growable hash table using open addressing and linear probing.
 *
 * Each element is a non-null void pointer with a fixed hash. The hash table
 * stores both elements and hashes so it can resize the table dynamically as
 * needed.
 *
 * This can be used as a set or a map. When used as a set, you can simply use
 * otable_put(), otable_contains() and otable_remove() to manage the contents.
 * When used as a map, to find an element by key you must loop over the
 * collision chain for the key's hash; see otable_find() for an example.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * A bucket.
 *
 * The bucket is empty if and only if the element is NULL. (An empty bucket has
 * indeterminate hash.)
 *
 * Hashes are stored so that the hash table may grow dynamically as needed.
 */
typedef struct otable_bucket_t {
    void* /*nullable*/ element;
    uint32_t hash;
} otable_bucket_t;

/**
 * The open hash table.
 */
typedef struct otable_t {
    otable_bucket_t* buckets;
    size_t bits;
    size_t count;
} otable_t;

/**
 * Creates an open hash table.
 */
otable_t* otable_new(void);

/**
 * Creates a copy of an existing table.
 */
otable_t* otable_new_copy(otable_t* table);

/**
 * Deletes an open hash table.
 */
void otable_delete(otable_t* otable);

/**
 * Puts a new element with the given hash in the hashtable if it does not
 * already exist.
 *
 * Returns true if the element was inserted; false if it already existed in the
 * table.
 *
 * You retain ownership over any entries placed in the hashtable. You must free
 * them yourself.
 */
bool otable_put(otable_t* otable, void* element, uint32_t hash);

/**
 * Returns true if the given element exists; false otherwise.
 */
bool otable_contains(otable_t* otable, void* element, uint32_t hash);

/**
 * Reserves a number of buckets equal to two to the power of the given number
 * of bits.
 */
void otable_reserve_bits(otable_t* otable, size_t new_bits);

/**
 * Removes an element.
 *
 * Returns true if the element was removed, false if it didn't exist.
 *
 * When an element is removed, colliding elements are moved back as necessary
 * (there are no tombstones.)
 *
 * This does not reduce the size of the table. Call table_shrink() afterwards
 * to reclaim space if desired.
 */
bool otable_remove(otable_t* otable, void* element, uint32_t hash);

/**
 * Reduces the size of the table if possible.
 *
 * If the table has shrunk significantly from its largest size, this can reduce
 * the size to something more reasonable for the current contents. If the table
 * is already the correct size, this does nothing. It is safe to call this
 * after every removal if you always want to keep the table a reasonable size.
 *
 * Call this after removing entries if you'd like to reclaim space.
 */
void otable_shrink(otable_t* otable);

/**
 * Returns a pointer to the first element in the collision chain for the given
 * hash, or NULL if there are no elements with this hash.
 *
 * You must verify that the returned element is the one you want (for example,
 * if your otable is a map, you must check that the returned element has the
 * correct key.) If not, you must call otable_collision() to get the next
 * element in the collision chain.
 *
 * Here's one example of how you might do this:
 *
 *     for (void** entry = otable_find(otable, hash); entry;
 *             entry = otable_collision(otable, entry, hash))
 *     {
 *         foo_t* element = *entry;
 *         if (foo_key_matches(element, key)) {
 *             // element found
 *             return;
 *         }
 *     }
 *     // element not found
 *
 * This has to be done instead of using object-like macros or function pointers
 * for key lookup because we don't have either of those in our bootstrapping
 * compilers.
 */
void** otable_find(otable_t* otable, uint32_t hash);

/**
 * Returns a pointer to the next element in the collision chain for the given
 * hash, or NULL if there are no more elements with this hash.
 *
 * The entry parameter is that previously returned from otable_find() or
 * otable_collision().
 *
 * See otable_find().
 */
void** otable_collision(otable_t* otable, void** entry, uint32_t hash);

/**
 * Returns a pointer to the first element in the table, or NULL if the table is
 * empty.
 *
 * Call otable_next() to find subsequent elements.
 *
 * This can be used to iterate over the contents of the table. For example:
 *
 *     for (void** p = otable_begin(otable); p; p = otable_next(otable, p)) {
 *         foo_t* foo = *p;
 *         // ...
 *     }
 */
void** otable_begin(otable_t* otable);

/**
 * Returns a pointer to the next element in the table, or NULL if there are no
 * more elements.
 *
 * The entry parameter is that previously returned from otable_begin() or
 * otable_next().
 */
void** otable_next(otable_t* otable, void** entry);

/**
 * Returns the number of entries in the hashtable.
 */
#ifndef __onramp_cpp_omc__
    #ifndef DEBUG
        #define otable_count(otable) ((otable)->count)
    #endif
#endif
#ifndef otable_count
    static inline size_t otable_count(otable_t* otable) {
        return otable->count;
    }
#endif

/**
 * Returns true if the table is empty, false otherwise.
 */
#ifndef __onramp_cpp_omc__
    #ifndef DEBUG
        #define otable_is_empty(otable) ((otable)->count == 0)
    #endif
#endif
#ifndef otable_is_empty
    static inline bool otable_is_empty(otable_t* otable) {
        return otable->count == 0;
    }
#endif

/**
 * Adds all elements of another table into this one.
 */
void otable_union(otable_t* restrict otable, const otable_t* restrict other);

/**
 * Removes all elements and frees the memory backing the table.
 */
void otable_remove_all(otable_t* otable);

#endif
