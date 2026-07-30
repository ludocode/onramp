/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2024-2026 Fraser Heavy Software
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

#include "hideset.h"

#include <assert.h>
#include <stdlib.h>

#include "libo-error.h"

// TODO this implementation is pretty inefficient due to all the individually
// allocated hideset entries. Most hidesets have at most one, maybe two
// entries. It would probably be a lot faster if we just made this an open
// hashtable.

void hideset_deref(hideset_t* hideset) {
    assert(hideset);
    if (--hideset->refcount)
        return;

    for (table_entry_t** bucket = table_first_bucket(&hideset->table); bucket;
            bucket = table_next_bucket(&hideset->table, bucket))
    {
        for (table_entry_t* entry = *bucket; entry;) {
            table_entry_t* next = table_entry_next(entry);
            string_deref(((hideset_entry_t*)entry)->string);
            free(entry);
            entry = next;
        }
    }
    table_destroy(&hideset->table);

    free(hideset);
}

static void hideset_add(hideset_t* hideset, string_t* string) {
    assert(hideset); // should have already been created
    hideset_entry_t* entry = malloc(sizeof(hideset_entry_t));
    entry->string = string_ref(string);
    table_put(&hideset->table, (table_entry_t*)entry, string_hash(string));
}

void hideset_add_all(hideset_t* hideset, const hideset_t* other) {
    for (table_entry_t** bucket = table_first_bucket(&((hideset_t*)other)->table); bucket;
            bucket = table_next_bucket(&((hideset_t*)other)->table, bucket))
    {
        for (table_entry_t* entry = *bucket; entry; entry = table_entry_next(entry)) {
            hideset_add(hideset, ((hideset_entry_t*)entry)->string);
        }
    }
}

static hideset_t* hideset_new_impl(void) {
    hideset_t* hideset = malloc(sizeof(hideset_t));
    hideset->refcount = 1;
    table_init(&hideset->table);
    return hideset;
}

hideset_t* hideset_new(hideset_t* /*nullable*/ old, string_t* string) {
    hideset_t* hideset = hideset_new_impl();

    // add the current macro to the hideset
    hideset_add(hideset, string);

    // copy entries from old hideset
    if (old) {
        hideset_add_all(hideset, old);
    }

    return hideset;
}

hideset_t* hideset_new_intersection(hideset_t* /*nullable*/ old, hideset_t*
        /*nullable*/ closing_paren, string_t* current)
{
    hideset_t* hideset = hideset_new_impl();

    // add the current macro to the hideset
    hideset_add(hideset, current);

    // if either hideset is null, the intersection is empty, so we're done
    if (!old || !closing_paren) {
        return hideset;
    }

    // for each entry in the old hideset, add it if it also exists in the
    // closing_paren hideset. (it's not possible for the current macro to be in
    // the old hideset so we don't need to check for duplicates.)
    for (table_entry_t** bucket = table_first_bucket(&old->table); bucket;
            bucket = table_next_bucket(&old->table, bucket))
    {
        for (table_entry_t* entry = *bucket; entry; entry = table_entry_next(entry)) {
            string_t* string = ((hideset_entry_t*)entry)->string;
            bool found = false;
            for (table_entry_t* paren_entry = table_bucket(&closing_paren->table, string_hash(string));
                    paren_entry; paren_entry = table_entry_next(paren_entry))
            {
                if (string_equal(((hideset_entry_t*)paren_entry)->string, string)) {
                    found = true;
                    break;
                }
            }
            if (found) {
                hideset_add(hideset, string);
            }
        }
    }

    return hideset;
}

hideset_t* hideset_new_union(hideset_t* left, hideset_t* right) {
    hideset_t* hideset = hideset_new_impl();
    hideset_add_all(hideset, left);
    hideset_add_all(hideset, right);
    return hideset;
}

bool hideset_contains(hideset_t* hideset, string_t* string) {
    for (table_entry_t* entry = table_bucket(&hideset->table, string_hash(string));
            entry; entry = table_entry_next(entry))
    {
        if (string_equal(((hideset_entry_t*)entry)->string, string)) {
            return true;
        }
    }
    return false;
}

void hideset_print(hideset_t* hideset) {
    putchar('{');
    bool first = true;
    for (table_entry_t** bucket = table_first_bucket(&hideset->table); bucket;
            bucket = table_next_bucket(&hideset->table, bucket))
    {
        for (table_entry_t* entry = *bucket; entry; entry = table_entry_next(entry)) {
            string_t* string = ((hideset_entry_t*)entry)->string;

            if (first) {
                first = false;
            } else {
                putchar(',');
            }
            fputs(string->bytes, stdout);
        }
    }
    putchar('}');
}
