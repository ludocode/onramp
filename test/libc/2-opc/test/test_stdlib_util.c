// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

// TODO
int main(void){}
#ifdef DISABLED
#include "stdlib_util.c"

#include "test.h"

#include <stdio.h>

static void test_stdlib_util_rand(void) {

    // generate some random numbers, make sure at least some of them differ
    char a[5];
    for (size_t i = 0; i < 5; ++i)
        a[i] = rand();
    libc_assert(
            a[0] != a[1] ||
            a[1] != a[2] ||
            a[2] != a[3] ||
            a[3] != a[4]);

    // make sure srand(1) gives us the same sequence again
    char b[5];
    srand(1);
    for (size_t i = 0; i < 5; ++i)
        b[i] = rand();
    libc_assert(
            a[0] == b[0] &&
            a[1] == b[1] &&
            a[2] == b[2] &&
            a[3] == b[3] &&
            a[4] == b[4]);

    // make sure srand(2) gives us some other sequence
    char c[5];
    srand(2);
    for (size_t i = 0; i < 5; ++i)
        c[i] = rand();
    libc_assert(
            a[0] != c[0] ||
            a[1] != c[1] ||
            a[2] != c[2] ||
            a[3] != c[3] ||
            a[4] != c[4]);

}

static int test_stdlib_util_bsearch_compare_int(const void* vx, const void* vy) {
    int x = *(const int*)vx;
    int y = *(const int*)vy;
    return (x < y) ? -1 : (x > y) ? 1 : 0;
}

static void test_stdlib_util_bsearch(void) {
    int a[] = {2, 4, 5, 8, 13, 13, 16, 19, 22};

    // test some values that exist
    int x2 = 2;
    test_assert(a == bsearch(&x2, a, sizeof(a)/sizeof(*a), sizeof(int),
                test_stdlib_util_bsearch_compare_int));
    int x8 = 8;
    test_assert(a + 3 == bsearch(&x8, a, sizeof(a)/sizeof(*a), sizeof(int),
                test_stdlib_util_bsearch_compare_int));
    int x22 = 22;
    test_assert(a + 8 == bsearch(&x22, a, sizeof(a)/sizeof(*a), sizeof(int),
                test_stdlib_util_bsearch_compare_int));

    // test some values that don't exist
    int x1 = 1;
    test_assert(NULL == bsearch(&x1, a, sizeof(a)/sizeof(*a), sizeof(int),
                test_stdlib_util_bsearch_compare_int));
    int x6 = 6;
    test_assert(NULL == bsearch(&x6, a, sizeof(a)/sizeof(*a), sizeof(int),
                test_stdlib_util_bsearch_compare_int));
    int x99 = 99;
    test_assert(NULL == bsearch(&x99, a, sizeof(a)/sizeof(*a), sizeof(int),
                test_stdlib_util_bsearch_compare_int));
}

void test_stdlib_util() {
    test_stdlib_util_rand();
    test_stdlib_util_bsearch();
    puts("stdlib util tests pass");
}
#endif
