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

#include "libo-table.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "libo-error.h"
#include "libo-util.h"

void table_init(table_t* table) {
    table->bits = 0;
    table->count = 0;
    table->entries.single_bucket = NULL;
}

void table_destroy(table_t* table) {
    if (table->bits > 0) {
        free(table->entries.buckets);
    }
}

table_t* table_new(void) {
    table_t* table = (table_t*) malloc(sizeof(table_t));
    if (table == NULL) {
        fatal("Out of memory.");
    }
    table_init(table);
    return table;
}

void table_delete(table_t* table) {
    table_destroy(table);
    free(table);
}

static void table_resize(table_t* table, int new_bits) {
    assert(table->bits != new_bits);
    if (new_bits > 30) {
        fatal("Out of memory.");
    }
    size_t old_capacity = 1 << table->bits;
    //printf("resizing %zi to %i\n",old_capacity,1<<new_bits);

    // if we're resizing to zero, flatten all entries into a single linked
    // list, ignoring the hashes.
    if (new_bits == 0) {
        table_entry_t** old_buckets = table->entries.buckets;
        table_entry_t** new_tail = &table->entries.single_bucket;
        size_t old_capacity = 1 << table->bits;
        for (size_t i = 0; i < old_capacity; ++i) {
            for (table_entry_t* entry = old_buckets[i]; entry; entry = entry->next) {
                *new_tail = entry;
                entry->previous = new_tail;
                new_tail = &entry->next;
            }
        }
        *new_tail = NULL;
        return;
    }

    // otherwise allocate new entries
    size_t new_capacity = 1 << new_bits;
    table_entry_t** new_buckets = calloc(new_capacity, sizeof(table_entry_t*));
    if (new_buckets == NULL) {
        fatal("Out of memory.");
    }

    // move all entries from old buckets to new ones
    table_entry_t** old_buckets = (table->bits == 0) ?
            &table->entries.single_bucket : table->entries.buckets;
    for (size_t i = 0; i < old_capacity; ++i) {
        for (table_entry_t* entry = old_buckets[i]; entry;) {
            table_entry_t* next = entry->next;

            // get bucket
            size_t j = knuth_hash_32(entry->hash, new_bits);
            table_entry_t** bucket = new_buckets + j;

            // insert entry at head of bucket
            if (*bucket) {
                (*bucket)->previous = &entry->next;
            }
            entry->next = *bucket;
            entry->previous = bucket;
            *bucket = entry;

            entry = next;
        }
    }

    // adopt new entries
    if (table->bits != 0) {
        free(table->entries.buckets);
    }
    table->entries.buckets = new_buckets;
    table->bits = new_bits;
}

void table_reserve_bits(table_t* table, int new_bits) {
    if (table->bits <= new_bits)
        return;
    table_resize(table, new_bits);
}

void table_put(table_t* table, table_entry_t* entry, uint32_t hash) {

    // if our hashtable is at capacity, grow by 4x
    if (table->count > 4 + (1 << table->bits)) {
        table_resize(table, table->bits + 2);
    }
    ++table->count;

    // find bucket
    table_entry_t** bucket;
    if (table->bits == 0) {
        bucket = &table->entries.single_bucket;
        //printf("inserting into %p entry %p hash %u single bucket\n", (void*)table, (void*)entry, hash);
    } else {
        bucket = &table->entries.buckets[knuth_hash_32(hash, table->bits)];
        //printf("inserting into %p entry %p hash %u bucket %u\n", (void*)table, (void*)entry, hash, knuth_hash_32(hash, table->bits));
    }

    // insert at front of list
    entry->hash = hash;
    entry->next = *bucket;
    entry->previous = bucket;
    if (entry->next) {
        entry->next->previous = &entry->next;
    }
    *bucket = entry;
}

table_entry_t** table_first_bucket(table_t* table) {
    if (table->bits == 0) {
        return &table->entries.single_bucket;
    }
    return table->entries.buckets;
}

table_entry_t** table_next_bucket(table_t* table, table_entry_t** bucket) {
    if (table->bits == 0) {
        return NULL;
    }
    if (++bucket == table->entries.buckets + (1 << table->bits)) {
        return NULL;
    }
    return bucket;
}

table_entry_t* table_bucket(table_t* table, uint32_t hash) {
    if (table->bits == 0) {
        //printf("table %p returning hash %u single bucket\n", (void*)table, hash);
        return table->entries.single_bucket;
    }
    //printf("table %p returning hash %u bucket %u\n", (void*)table, hash, knuth_hash_32(hash, table->bits));
    return table->entries.buckets[knuth_hash_32(hash, table->bits)];
}

void table_remove(table_t* table, table_entry_t* entry) {
    assert(table != NULL);
    assert(entry != NULL);
    assert(table->count > 0);
    --table->count;

    if (entry->next) {
        entry->next->previous = entry->previous;
    }
    *entry->previous = entry->next;

    // if our hashtable capacity is more than 8x count, shrink to 1/4 capacity
    if (table->bits >= 4 && table->count < (1 << (table->bits - 3))) {
        table_resize(table, table->bits - 2);
    }
}
