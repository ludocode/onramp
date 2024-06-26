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

#ifndef ONRAMP_LIBO_TABLE_H_INCLUDED
#define ONRAMP_LIBO_TABLE_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

/**
 * @file
 *
 * A simple intrusive growable hashtable.
 *
 * The hashtable uses closed hashing with linked lists for collision
 * resolution. It initially uses a single bucket so it starts out as a simple
 * linked list. The number of buckets grows automatically as it fills and
 * shrinks as it empties. Buckets store their collision list as an asymmetric
 * doubly-linked list to make it as fast and simple as possible to remove
 * elements from the table.
 *
 * Some Onramp components use large numbers of maps and sets. For example, in
 * the preprocessor, each expansion token needs its own set to store which
 * macros were used to generate it. A growable map is necessary to store and
 * access these efficiently; this hashtable is one of the simplest ways to do
 * it.
 *
 * In order to store a value in one of these tables, it must contain the entry
 * struct (table_entry_t.) The address of the entry field is passed to the
 * hashtable to insert it. containerof() is used to convert pointers to the
 * entry back to the containing struct.
 *
 * To find a value in the hashtable, table_bucket() is used to retrieve the
 * bucket that, if the value exists, would contain it. The contents of the
 * bucket must be iterated over manually. (opC doesn't have function pointers
 * so there's no simple way to make a type-generic lookup function.)
 */

/**
 * An entry in the hashtable.
 */
typedef struct table_entry_t {
    struct table_entry_t** previous;
    struct table_entry_t* next;
    uint32_t hash;
} table_entry_t;

/**
 * The hashtable.
 */
typedef struct table_t {
    union {
        table_entry_t** buckets;
        table_entry_t* single_bucket;
    } entries;
    int bits;
    int count;
} table_t;

/**
 * Allocates and initializes a hashtable.
 *
 * Call table_delete() when done.
 */
table_t* table_new(void);

/**
 * Initializes the hashtable.
 *
 * Call table_destroy() when done.
 */
void table_init(table_t* table);

/**
 * Destroys the given hashtable that was created with table_init().
 *
 * This does not free any entries in the hashtable. You own the entries; you
 * must free them yourself.
 */
void table_destroy(table_t* table);

/**
 * Destroys and de-allocates the given hashtable that was created with
 * table_new().
 *
 * This does not free any entries in the hashtable. You own the entries; you
 * must free them yourself.
 */
void table_delete(table_t* table);

/**
 * Reserves a number of buckets equal to two to the power of the given number
 * of bits.
 */
void table_reserve_bits(table_t* table, int new_bits);

/**
 * Returns the number of entries in the hashtable.
 */
#ifndef __onramp_cpp_omc__
    #ifndef DEBUG
        #define table_count(table) ((table)->count)
    #endif
#endif
#ifndef table_count
    static inline size_t table_count(table_t* table) {
        return table->count;
    }
#endif

/**
 * Puts a new entry with the given hash in the hashtable.
 *
 * You retain ownership over any entries placed in the hashtable. You must free
 * them yourself.
 */
void table_put(table_t* table, table_entry_t* entry, uint32_t hash);

/**
 * Returns a pointer to the list of entries in the first bucket.
 *
 * Dereference this to get the first element in the bucket and call
 * table_entry_next() to walk the bucket. Call table_next_bucket() to advance
 * to the next bucket.
 */
table_entry_t** table_first_bucket(table_t* table);

/**
 * Returns a pointer to the bucket following that given, or NULL if there are
 * no more buckets.
 */
table_entry_t** table_next_bucket(table_t* table, table_entry_t** bucket);

/**
 * Returns the first element in the bucket for the given hash, or NULL if it is
 * empty.
 *
 * Use this to search for an element in the table. Call table_entry_next() to
 * walk the contents of the bucket.
 */
table_entry_t* table_bucket(table_t* table, uint32_t hash);

/**
 * Returns the hash of the given entry.
 */
#ifndef __onramp_cpp_omc__
    #ifndef DEBUG
        #define table_entry_hash(entry) ((entry)->hash)
    #endif
#endif
#ifndef table_entry_hash
    static inline uint32_t table_entry_hash(table_entry_t* entry) {
        return entry->hash;
    }
#endif

/**
 * Returns the entry following this one in the same hashtable bucket, or NULL
 * if there is none.
 */
#ifndef __onramp_cpp_omc__
    #ifndef DEBUG
        #define table_entry_next(entry) ((entry)->next)
    #endif
#endif
#ifndef table_entry_next
    static inline table_entry_t* table_entry_next(table_entry_t* entry) {
        return entry->next;
    }
#endif

/**
 * Removes the given entry from the table.
 */
void table_remove(table_t* table, table_entry_t* entry);

#endif
