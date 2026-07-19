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

#include "libo-otable.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "libo-util.h"
#include "libo-error.h"

// Some of our tools use a lot of small tables, in particular cpp/2 (a hideset
// table per token) and cg/1 (a liveness table per block repeatedly copied and
// merged.) We should therefore keep this fairly small. 3 bits is 8 buckets for
// a total of 64 bytes, allowing up to 4 elements without resizing. (Buckets
// are only allocated if the map is not empty.)
#define OTABLE_MINIMUM_BITS 3

otable_t* otable_new(void) {
    return calloc(1, sizeof(otable_t));
}

void otable_delete(otable_t* otable) {
    free(otable->buckets);
    free(otable);
}

otable_t* otable_new_copy(otable_t* otable) {
    otable_t* copy = otable_new();
    if (!copy) {
        fatal("Out of memory.");
    }
    if (otable->buckets) {
        copy->buckets = __memdup(otable->buckets, (1u << otable->bits) * sizeof(otable_bucket_t));
        if (!copy->buckets) {
            fatal("Out of memory.");
        }
        copy->bits = otable->bits;
        copy->count = otable->count;
    }
    return copy;
}

static void otable_resize(otable_t* otable, size_t new_bits) {
    //printf("otable resize to %zi bits %zi buckets\n",new_bits, (size_t)1u<<new_bits);
    assert(otable);
    otable_bucket_t* old_buckets = otable->buckets;
    size_t old_bits = otable->bits;

    // safety checks
    assert(old_bits != new_bits);
    if (new_bits > 30) {
        fatal("Out of memory.");
    }
    assert((1u << new_bits) >= (otable->count << 1)); // max half full

    // allocate new buckets
    size_t new_capacity = 1u << new_bits;
    otable_bucket_t* new_buckets = malloc(new_capacity * sizeof(otable_bucket_t));
    if (new_buckets == NULL) {
        fatal("Out of memory.");
    }
    for (size_t i = 0; i < new_capacity; ++i) {
        new_buckets[i].element = NULL;
    }

    // copy old items to new
    if (old_buckets) {
        size_t old_capacity = 1u << old_bits;
        size_t new_mask = new_capacity - 1;
        for (size_t i = 0; i < old_capacity; ++i) {
            otable_bucket_t* old_bucket = old_buckets + i;
            if (old_bucket->element != NULL) {
                size_t j = knuth_hash_32(old_bucket->hash, new_bits);
                for (;;) {
                    otable_bucket_t* new_bucket = new_buckets + j;
                    if (new_bucket->element == NULL)  {
                        new_bucket->element = old_bucket->element;
                        new_bucket->hash = old_bucket->hash;
                        break;
                    }
                    assert(new_bucket->element != old_bucket->element); // no duplicates
                    j = (j + 1) & new_mask;
                }
            }
        }
    }

    // replace old items
    free(old_buckets);
    otable->buckets = new_buckets;
    otable->bits = new_bits;
}

/**
 * Find the bucket containing this element, or the bucket where this element
 * should be inserted if it does not exist.
 *
 * The table must have a non-zero size before calling this.
 */
static otable_bucket_t* otable_bucket(otable_t* otable, void* element, uint32_t hash) {
    assert(otable->bits);

    size_t bits = otable->bits;
    size_t mask = (1u << bits) - 1;
    size_t i = knuth_hash_32(hash, bits);
    for (;;) {
        otable_bucket_t* bucket = otable->buckets + i;
        if (bucket->element == NULL || bucket->element == element) {
            return bucket;
        }
        i = (i + 1) & mask;
    }
}

bool otable_put(otable_t* otable, void* element, uint32_t hash) {

    // make sure we have at least some space in the table
    if (otable->bits == 0) {
        otable_resize(otable, OTABLE_MINIMUM_BITS);
    }

    // check if the element is in the table
    otable_bucket_t* bucket = otable_bucket(otable, element, hash);
    if (bucket->element != NULL) {
        // element already exists.
        assert(bucket->element == element);
        return false;
    }

    // insert the element
    //printf("put %c at %zi\n",*(char*)element, bucket - otable->buckets);
    bucket->element = element;
    bucket->hash = hash;
    ++otable->count;

    // if the element count just went above half the bucket count, grow the
    // table
    if ((otable->count << 1) > (1u << otable->bits)) {
        otable_resize(otable, otable->bits + 1);
    }

    return true;
}

bool otable_contains(otable_t* otable, void* element, uint32_t hash) {
    if (otable->bits == 0) {
        return false;
    }
    return otable_bucket(otable, element, hash)->element != NULL;
}

bool otable_remove(otable_t* otable, void* element, uint32_t hash) {
    //printf("removing %c\n",*(char*)element);
    if (otable->bits == 0) {
        return false;
    }
    otable_bucket_t* bucket = otable_bucket(otable, element, hash);
    //printf("found %c at %zi\n",*(char*)element, bucket - otable->buckets);
    if (bucket->element == NULL) {
        return false;
    }
    assert(bucket->element == element);
    --otable->count;

    otable_bucket_t* buckets = otable->buckets;
    size_t bits = otable->bits;

    // We can't just remove the element; we might be breaking a run of
    // collisions. We have to walk forward and check.
    size_t mask = (1u << bits) - 1;
    size_t i = bucket - buckets;
    size_t j = i;
    for (;;) {
        j = (j + 1) & mask;

        // i == j could only happen if the table was full, which is not
        // possible because we resize dynamically.
        assert(i != j);

        // Check for an empty bucket
        otable_bucket_t* other = buckets + j;
        if (other->element == NULL) {
            //printf("end of run at %zi. removing element at %zi\n",j, i);
            // We've found the end of the run. We can safely remove our
            // element.
            bucket->element = NULL;
            return true;
        }

        // Figure out where this element is supposed to be.
        size_t k = knuth_hash_32(other->hash, bits);
        //printf("element %c i %zi j %zi k %zi\n",*(char*)other->element,i, j, k);

        // This is the condition that tells us whether we can move an element
        // back. It is described in pseudocode here:
        //
        //     https://en.wikipedia.org/wiki/Open_addressing
        //
        //if ((i < j) ? (k > i && k <= j) : (k > i || k <= j)) {
        //
        // The element is currently at j; its ideal hash position would be k;
        // and we are trying to determine whether to move it back to the hole
        // being created at i.
        //
        // In the first case (i < j), if the hash k is between i and j, the
        // element at j is where it belongs (as j can be found from a collision
        // sequence starting at k.) If not it must be moved back to i.
        //
        //          ......i...k...j.....   j is where it belongs (after k)
        //          ...k..i.......j.....   j must be moved back to i (so it follows k)
        //          ......i.......j..k..   j must be moved back to i (so it follows k)
        //
        // The second case (i > j) is when we've wrapped around. In this case
        // if the hash k is after i or before j, the element is where it
        // belongs (again j following k); if not it must be moved back to i.
        //
        //          ...j......k.....i...   j must be moved back to i (so it follows k)
        //          .k.j............i...   j is where it belongs (after k)
        //          ...j............i.k.   j is where it belongs (after k)
        //
        // In both cases the check is exclusive of i and inclusive of j,
        // because if k==i, the element belongs exactly at i and must be moved
        // back, and if k==j, the element is already exactly where it goes and
        // must not be moved.
        //
        // This can be optimized into a sequence of xors; see:
        //
        //     https://stackoverflow.com/a/60709252
        //
        // This is not necessarily faster on Onramp since xor is three
        // primitive instructions but it is much less branching so the
        // bytecode is simpler.
        if ((i < j) ^ (k <= i) ^ (k > j)) {
            //printf("element %c at %zi is where it's supposed to be.\n",*(char*)other->element,j);
            // This element is where it's supposed to be. Continue to the
            // next one.
            continue;
        }

        // This element must be moved back.
        //printf("moving %c at %zi back to %zi.\n",*(char*)other->element,j,i);
        bucket->element = other->element;
        bucket->hash = other->hash;

        // Keep searching from the new location.
        bucket = other;
        i = j;
    }
}

void otable_reserve_bits(otable_t* otable, size_t new_bits) {
    if (otable->bits >= new_bits)
        return;
    otable_resize(otable, new_bits);
}

void otable_union(otable_t* restrict otable, const otable_t* restrict other) {
    assert(otable);
    assert(other);
    assert(otable != other);
    if (other->count == 0) {
        return;
    }
    size_t other_capacity = 1u << other->bits;
    for (size_t i = 0; i < other_capacity; ++i) {
        otable_bucket_t* bucket = other->buckets + i;
        if (bucket->element != NULL) {
            otable_put(otable, bucket->element, bucket->hash);
        }
    }
}

void otable_shrink(otable_t* otable) {
    if (otable->count == 0) {
        free(otable->buckets);
        otable->buckets = NULL;
        otable->bits = 0;
        return;
    }

    size_t bits = otable->bits;
    size_t load = otable->count << 2u;
    //printf("bits %zu count %zu\n", bits, otable->count);
    while ((1u << bits) > load && bits > OTABLE_MINIMUM_BITS) {
        --bits;
    }
    if (bits != otable->bits) {
        //printf("shrinking! from %zu bits to %zu bits\n", otable->bits, bits);
        otable_resize(otable, bits);
    }
}

void otable_remove_all(otable_t* otable) {
    free(otable->buckets);
    otable->buckets = NULL;
    otable->bits = 0;
    otable->count = 0;
}

static void** otable_find_impl(otable_t* otable, size_t i, uint32_t hash) {
    assert(otable->count != 0);
    otable_bucket_t* buckets = otable->buckets;
    size_t mask = (1u << otable->bits) - 1;
    for (;;) {
        otable_bucket_t* bucket = buckets + i;
        if (!bucket->element) {
            // end of collision chain
            return NULL;
        }
        if (bucket->hash == hash) {
            // matching hash found
            return &bucket->element;
        }
        // colliding element found but hash differs. keep walking.
        i = (i + 1) & mask;
    }
}

void** otable_find(otable_t* otable, uint32_t hash) {
    if (otable->count == 0) {
        return NULL;
    }
    size_t i = knuth_hash_32(hash, otable->bits);
    return otable_find_impl(otable, i, hash);
}

void** otable_collision(otable_t* otable, void** entry, uint32_t hash) {
    assert(otable->count != 0);
    size_t i = (otable_bucket_t*)entry - otable->buckets;
    size_t mask = (1u << otable->bits) - 1;
    i = (i + 1) & mask;
    return otable_find_impl(otable, i, hash);
}

void** otable_next_impl(otable_t* otable, otable_bucket_t* p) {
    otable_bucket_t* end = otable->buckets + (1u << otable->bits);
    for (; p != end; ++p) {
        if (p->element) {
            return &p->element;
        }
    }
    return NULL;
}

void** otable_begin(otable_t* otable) {
    if (otable->count == 0) {
        return NULL;
    }
    return otable_next_impl(otable, otable->buckets);
}

void** otable_next(otable_t* otable, void** entry) {
    return otable_next_impl(otable, (otable_bucket_t*)entry + 1);
}
