// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.


// We rename malloc and free for testing purposes because glibc uses them quite
// a bit in its implementation, and we aren't overriding several functions
// like calloc() which will cause conflicts.
#include <stdlib.h>
#undef malloc
#undef free
#undef realloc
#define malloc omc_malloc
#define free omc_free
#define realloc omc_realloc
#define __onramp_libc1_test
#include "stdlib_malloc.c"
#undef malloc
#undef free
#undef realloc

//#include "stdlib_malloc.c"

#include "test.h"

#include <string.h>

/**
 * Verifies that the only thing in the free list is a single allocation the
 * full size of the heap.
 */
static void test_stdlib_malloc_check_heap_empty(void) {
    test_assert(free_list != 0); // there is a first allocation
    int* alloc = free_list;
    test_assert(*alloc == 0); // there is no next allocation
    //printf("%i %i\n", *(alloc - 1) * 4 ,(int)(heap_end - heap_start) - 4);
    test_assert((*(alloc - 1) * 4) == ((int)(heap_end - heap_start) - 4)); // it's the full heap
}

/**
 * Walk the free list checking that each allocation is properly ordered,
 * non-adjacent and non-overlapping.
 */
static void test_stdlib_malloc_sanity_check(void) {
    int* alloc = free_list;
    while (alloc) {
        test_assert((char*)alloc > heap_start); // start is in bounds
        int size = *(alloc - 1);
        char* end = (char*)alloc + size;
        test_assert(end <= heap_end); // end is in bounds
        alloc = (int*)*alloc; // next
        if (alloc != 0) {
            test_assert((char*)alloc > end); // next comes after us
        }
    }
}

/**
 * Prints the contents of the free list.
 */
#ifdef __GNUC__
__attribute__((unused))
#endif
static void test_stdlib_malloc_print_free_list(void) {
    printf("-- heap size: %zi\n", (size_t)(heap_end - heap_start));

    int* alloc = free_list;
    while (alloc) {
        printf("---- free alloc at %p size %i\n", alloc, *(alloc-1));
        if (*alloc != 0 && *alloc <= (int)alloc) {
            printf("---- ERROR heap loop or sort problem\n");
            break;
        }
        alloc = (int*)*alloc; // next
    }
}

static void test_stdlib_malloc_free_null(void) {
    test_stdlib_malloc_check_heap_empty();
    omc_free(NULL);
    test_stdlib_malloc_check_heap_empty();
}

static void test_stdlib_malloc_entire_heap(void) {
    test_stdlib_malloc_check_heap_empty();

    char* p = (char*)omc_malloc(heap_end-heap_start-sizeof(int*));
    test_stdlib_malloc_sanity_check();
    memcpy(p, "Hello", 5);

    omc_free(p);
    test_stdlib_malloc_check_heap_empty();
}

static void test_stdlib_malloc_merge_right(void) {
    test_stdlib_malloc_check_heap_empty();

    char* p = (char*)omc_malloc(5);
    test_stdlib_malloc_sanity_check();
    memcpy(p, "Hello", 5);

    omc_free(p);
    test_stdlib_malloc_check_heap_empty();
}

static void test_stdlib_malloc_merge_left(void) {
    test_stdlib_malloc_check_heap_empty();

    char* p = (char*)omc_malloc(32);
    test_stdlib_malloc_sanity_check();

    char* q = (char*)omc_malloc(heap_end-heap_start-32-2*sizeof(int*));
    test_stdlib_malloc_sanity_check();

    omc_free(p);
    omc_free(q);
    test_stdlib_malloc_check_heap_empty();
}

static void test_stdlib_malloc_merge_both(void) {
    test_stdlib_malloc_check_heap_empty();

    char* p = (char*)omc_malloc(5);
    test_stdlib_malloc_sanity_check();
    memcpy(p, "Hello", 5);

    char* q = (char*)omc_malloc(300);
    test_stdlib_malloc_sanity_check();

    omc_free(p);
    test_stdlib_malloc_sanity_check();
    omc_free(q);

    test_stdlib_malloc_check_heap_empty();
}

static void test_stdlib_malloc_insert_middle(void) {
    test_stdlib_malloc_check_heap_empty();

    char* p1 = (char*)omc_malloc(5);
    test_stdlib_malloc_sanity_check();
    char* p2 = (char*)omc_malloc(50);
    test_stdlib_malloc_sanity_check();
    char* p3 = (char*)omc_malloc(500);
    test_stdlib_malloc_sanity_check();
    char* p4 = (char*)omc_malloc(5000);
    test_stdlib_malloc_sanity_check();

    omc_free(p1);
    test_stdlib_malloc_sanity_check();
    omc_free(p3); // this one inserts between two others in the free list
//test_stdlib_malloc_print_free_list();
    test_stdlib_malloc_sanity_check();
    omc_free(p4);
    test_stdlib_malloc_sanity_check();
    omc_free(p2);

    test_stdlib_malloc_check_heap_empty();
}

void test_stdlib_malloc() {
    test_stdlib_malloc_check_heap_empty();
    test_stdlib_malloc_sanity_check();

    test_stdlib_malloc_free_null();

    test_stdlib_malloc_entire_heap();
    test_stdlib_malloc_merge_right();
    test_stdlib_malloc_merge_left();
    test_stdlib_malloc_merge_both();
    test_stdlib_malloc_insert_middle();

    puts("stdlib malloc tests pass");
}
