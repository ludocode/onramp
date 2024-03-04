#include "stdlib_malloc.c"

#include "test.h"

#include <stdbool.h>

static void test_stdlib_malloc_highest_bit(void) {
    test_assert(0 == highest_bit(0));

    for (int i = 0; i < (int)sizeof(size_t) * 8; ++i) {
        size_t x = 1 << i;
        //printf("%u %u %u\n", i, (int)x, highest_bit(x));
        test_assert(i == highest_bit(x));
        x |= x >> 1;
        //printf("%u %u %u\n", i, (int)x, highest_bit(x));
        test_assert(i == highest_bit(x));
        x |= 1;
        //printf("%u %u %u\n", i, (int)x, highest_bit(x));
        test_assert(i == highest_bit(x));
    }
}

/**
 * Verifies that the only thing in the free list is a single allocation the
 * full size of the heap.
 */
static void test_stdlib_malloc_check_heap_empty(void) {
    size_t full_size = (__heap_end - __heap_start) - TAGS_SIZE;
    size_t full_size_class = highest_bit(full_size);
    for (size_t i = 0; i < sizeof(size_t) * 8; ++i) {
        if (i == full_size_class) {
            free_alloc_t* full = free_list[i];
            test_assert(full->next == NULL);
            test_assert(full->previous == NULL);
            /*
            printf("%i %i %i %i %i %i %i %i\n",
                    __heap_start,
                    __heap_end,
                    full,
                    full_size,
                    *(size_t*)__heap_start,
                    *((size_t*)__heap_end - 1),
                    HEADER_TAG(full),
                    FOOTER_TAG(full, full_size));
                    */
            test_assert(HEADER_TAG(full) == full_size);
            test_assert(FOOTER_TAG(full, full_size) == full_size);
        } else {
            test_assert(free_list[i] == NULL);
        }
    }
}

/**
 * Walks the heap, checking that each allocation is valid and everything in the
 * free lists is accounted for.
 */
static void test_stdlib_malloc_sanity_check(size_t total_live) {
    if (total_live == 0) {
        // shortcut for when the entire heap is expected to be empty
        test_stdlib_malloc_check_heap_empty();
        // except we also do the full sanity tests anyway...
    }

    // First count the total number of free allocations
    size_t total_free = 0;
    for (size_t i = 0; i < sizeof(size_t) * 8; ++i) {
        for (free_alloc_t* alloc = free_list[i]; alloc != NULL; alloc = alloc->next) {
            ++total_free;
        }
    }

    // Walk the heap
    size_t heap_size = (size_t)(__heap_end - __heap_start);
    size_t count_live = 0;
    size_t count_free = 0;
    char* ptr = __heap_start + sizeof(size_t);
    while (true) {
        size_t tag = HEADER_TAG(ptr);
        size_t size = tag & ~1;
        test_assert(tag == FOOTER_TAG(ptr, size)); // make sure tags match
        test_assert((size & (MINIMUM_ALLOCATION_SIZE - 1)) == 0); // make sure it's a multiple of the minimum size
        test_assert(size + (ptr - __heap_start) + sizeof(size_t) <= heap_size); // make sure it's in bounds

        if ((tag & 1) == 0) {
            // The allocation is free. Make sure it exists in the correct free list.
            ++count_free;
            free_alloc_t* alloc = (free_alloc_t*)ptr;
            while (alloc->previous != NULL) {
                test_assert(alloc->previous->next == alloc);
                alloc = alloc->previous;
            }
            test_assert(alloc == free_list[highest_bit(size)]);
        } else {
            ++count_live;
        }

        // Move to the next allocation
        ptr += size + sizeof(size_t);
        if (ptr == __heap_end)
            break;
        ptr += sizeof(size_t);
    }

    // Make sure we have the correct number of allocations
    test_assert(count_free == total_free);
    test_assert(count_live == total_live);
}

/**
 * Prints the entire contents of the heap.
 */
#ifdef __GNUC__
__attribute__((unused))
#endif
static void test_stdlib_malloc_print_heap(void) {
    printf("-- heap size: %zi\n", (size_t)(__heap_end - __heap_start));
    char* ptr = __heap_start + sizeof(size_t);
    while (true) {
        size_t tag = HEADER_TAG(ptr);
        size_t size = tag & ~1;
        printf("---- alloc size %zi %s\n", size, (tag & 1) ? "in use" : "free");
        ptr += size + sizeof(size_t);
        if (ptr == __heap_end)
            break;
        ptr += sizeof(size_t);
    }
}

static void test_stdlib_malloc_free_null(void) {
    test_stdlib_malloc_sanity_check(0);
    free(NULL);
    test_stdlib_malloc_sanity_check(0);
}

static void test_stdlib_malloc_entire_heap(void) {
    test_stdlib_malloc_sanity_check(0);

    size_t size = (size_t)(__heap_end - __heap_start) - 2 * sizeof(size_t);
    char* p = (char*)malloc(size);
    test_stdlib_malloc_sanity_check(1);
    memcpy(p, "Hello", 5);
    p[size - 1] = 'a';

    free(p);
    test_stdlib_malloc_sanity_check(0);
}

static void test_stdlib_malloc_trivial(void) {
    test_stdlib_malloc_sanity_check(0);

    char* p = (char*)malloc(5);
    test_stdlib_malloc_sanity_check(1);
    memcpy(p, "Hello", 5);

    free(p);
    test_stdlib_malloc_sanity_check(0);
}

static void test_stdlib_malloc_in_order_free(void) {
    test_stdlib_malloc_sanity_check(0);

    char* p1 = (char*)malloc(5);
    test_stdlib_malloc_sanity_check(1);
    memcpy(p1, "Hello", 5);

    char* p2 = (char*)malloc(31);
    test_stdlib_malloc_sanity_check(2);
    memcpy(p2, "Hello", 5);

    char* p3 = (char*)malloc(77);
    test_stdlib_malloc_sanity_check(3);
    memcpy(p3, "Hello", 5);

    free(p1);
    test_stdlib_malloc_sanity_check(2);
    free(p2);
    test_stdlib_malloc_sanity_check(1);
    free(p3);
    test_stdlib_malloc_sanity_check(0);
}

static void test_stdlib_malloc_reverse_order_free(void) {
    test_stdlib_malloc_sanity_check(0);

    char* p1 = (char*)malloc(64);
    test_stdlib_malloc_sanity_check(1);
    memcpy(p1, "Hello", 5);

    char* p2 = (char*)malloc(0);
    test_stdlib_malloc_sanity_check(2);

    char* p3 = (char*)malloc(32);
    test_stdlib_malloc_sanity_check(3);
    memcpy(p3, "Hello", 5);

    free(p3);
    test_stdlib_malloc_sanity_check(2);
    free(p2);
    test_stdlib_malloc_sanity_check(1);
    free(p1);
    test_stdlib_malloc_sanity_check(0);
}

static void test_stdlib_malloc_out_of_order_free_1(void) {
    test_stdlib_malloc_sanity_check(0);

    char* p1 = (char*)malloc(5);
    test_stdlib_malloc_sanity_check(1);
    memcpy(p1, "Hello", 5);

    char* p2 = (char*)malloc(31);
    test_stdlib_malloc_sanity_check(2);
    memcpy(p2, "Hello", 5);

    char* p3 = (char*)malloc(77);
    test_stdlib_malloc_sanity_check(3);
    memcpy(p3, "Hello", 5);

    free(p2);
    test_stdlib_malloc_sanity_check(2);
    free(p1);
    test_stdlib_malloc_sanity_check(1);
    free(p3);
    test_stdlib_malloc_sanity_check(0);
}

static void test_stdlib_malloc_out_of_order_free_2(void) {
    test_stdlib_malloc_sanity_check(0);

    char* p1 = (char*)malloc(5);
    test_stdlib_malloc_sanity_check(1);
    memcpy(p1, "Hello", 5);

    char* p2 = (char*)malloc(31);
    test_stdlib_malloc_sanity_check(2);
    memcpy(p2, "Hello", 5);

    char* p3 = (char*)malloc(77);
    test_stdlib_malloc_sanity_check(3);
    memcpy(p3, "Hello", 5);

    free(p2);
    test_stdlib_malloc_sanity_check(2);
    free(p3);
    test_stdlib_malloc_sanity_check(1);
    free(p1);
    test_stdlib_malloc_sanity_check(0);
}

static void test_stdlib_malloc_stress_linked_lists(void) {
    test_stdlib_malloc_sanity_check(0);

    char* p[64];
    size_t total = 0;

    // allocate lots
    for (size_t i = 0; i < 64; ++i) {
        p[i] = (char*)malloc(32);
        test_stdlib_malloc_sanity_check(++total);
    }

    // free even
    for (size_t i = 0; i < 64; i += 2) {
        free(p[i]);
        p[i] = NULL;
        test_stdlib_malloc_sanity_check(--total);
    }

    // free some odds in the middle, make sure it's correctly removing from
    // linked lists
    for (size_t i = 11; i < 43; i += 2) {
        free(p[i]);
        p[i] = NULL;
        test_stdlib_malloc_sanity_check(--total);
    }

    // allocate some larger ones back
    for (size_t i = 30; i < 38; i += 2) {
        p[i] = (char*)malloc(64);
        test_stdlib_malloc_sanity_check(++total);
    }

    // free everything
    for (size_t i = 0; i < 64; ++i) {
        free(p[i]);
        if (p[i])
            test_stdlib_malloc_sanity_check(--total);
    }
    test_assert(total == 0);
}

void test_stdlib_malloc_realloc_grow_in_place() {
    test_stdlib_malloc_sanity_check(0);

    void* p = malloc(8);
    void* original_p = p;
    test_stdlib_malloc_sanity_check(1);
    strcpy(p, "Hello");

    p = realloc(p, 16);
    test_assert(p == original_p);
    test_stdlib_malloc_sanity_check(1);
    test_assert(0 == strcmp(p, "Hello"));
    strcpy(p, "Hello world!");

    p = realloc(p, 32);
    test_assert(p == original_p);
    test_stdlib_malloc_sanity_check(1);
    test_assert(0 == strcmp(p, "Hello world!"));

    free(p);
    test_stdlib_malloc_sanity_check(0);
}

void test_stdlib_malloc_realloc_grow_by_copy() {
    test_stdlib_malloc_sanity_check(0);

    void* p = malloc(8);
    void* original_p = p;
    test_stdlib_malloc_sanity_check(1);
    strcpy(p, "Hello");

    void* q = malloc(32);
    test_stdlib_malloc_sanity_check(2);

    p = realloc(p, 16);
    test_stdlib_malloc_sanity_check(2);
    test_assert(p != original_p);
    test_assert(0 == strcmp(p, "Hello"));

    free(q);
    test_stdlib_malloc_sanity_check(1);
    free(p);
    test_stdlib_malloc_sanity_check(0);
}

void test_stdlib_malloc() {
    test_stdlib_malloc_highest_bit();
    test_stdlib_malloc_sanity_check(0);

    test_stdlib_malloc_free_null();

    test_stdlib_malloc_entire_heap();
    test_stdlib_malloc_trivial();
    test_stdlib_malloc_in_order_free();
    test_stdlib_malloc_reverse_order_free();
    test_stdlib_malloc_out_of_order_free_1();
    test_stdlib_malloc_out_of_order_free_2();

    test_stdlib_malloc_stress_linked_lists();

    test_stdlib_malloc_realloc_grow_in_place();
    test_stdlib_malloc_realloc_grow_by_copy();

    puts("stdlib malloc tests pass");
}
