// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdlib.h>

int compare_ints(const void* vl, const void* vr) {
    const int* l = (const int*)vl;
    const int* r = (const int*)vr;
    if (*l < *r)
        return -1;
    if (*l > *r)
        return 1;
    return 0;
}

int main(void) {
    int x[] = {
        12, 8, 1, 29, 13, 26, 31, 27,
        32, 20, 30, 17, 5, 16, 3, 25,
        4, 11, 22, 6, 18, 15, 7, 23,
        2, 10, 19, 24, 9, 21, 14, 28,
    };

    qsort(x, sizeof(x) / sizeof(*x), sizeof(*x), compare_ints);

    for (size_t i = 1; i < sizeof(x) / sizeof(*x); ++i) {
        if (x[i - 1] >= x[i]) {
            return i;
        }
    }
}
