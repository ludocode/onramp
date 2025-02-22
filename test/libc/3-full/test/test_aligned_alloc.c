// The MIT License (MIT)
// Copyright (c) 2024-2025 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdlib.h>

#include <stdint.h>
#include <malloc.h>
#include <errno.h>

// These malloc tests have a lot of dependencies on the libc/3 malloc()
// implementation. They are not portable to other implementations.

#define TAGS_SIZE (2 * sizeof(size_t))

static void test_aligned_alloc(void) {

    // test an ordinary aligned allocation
    void* a = aligned_alloc(0x1000, 0x1000);
    if (a == NULL) exit(1); // ensure aligned
    if (malloc_usable_size(a) < 0x1000) exit(1); // ensure size
    if ((uintptr_t)a & 0xfff) exit(1); // ensure aligned
    free(a);

    // test an aligned allocation with a small size
    void* b = aligned_alloc(0x1000, 7);
    if ((uintptr_t)b & 0xfff) exit(2); // ensure aligned
    if (malloc_usable_size(b) >= 0x1000) exit(3); // ensure shrunk
    free(b);

    // test creation of some empty left area for alignment
    void* c = malloc(16);
    void* d = malloc(0x2000 - (((uintptr_t)c + 16 + 2 * TAGS_SIZE) & 0xfff) - 256); // misalign the heap by 256
    void* e = aligned_alloc(0x1000, 0x1000);
    if ((uintptr_t)e & 0xfff) exit(4); // ensure aligned
    void* f = aligned_alloc(64, 128); // allocate small enough to fit in the left area
    uintptr_t eu = (uintptr_t)e;
    uintptr_t fu = (uintptr_t)f;
    if (fu < (eu - 256) || fu >= eu) exit(5); // ensure it actually is left
    free(e);
    free(c);
    free(f);
    free(d);

    // test a forced jump to the next alignment when the left spacing is not
    // zero but not enough to fit metadata
    void* g = malloc(16);
    void* h = malloc(0x2000 - (((uintptr_t)g + 16 + 2 * TAGS_SIZE) & 0xfff) - 4); // misalign the heap by 4
    void* i = aligned_alloc(0x1000, 0x1000);
    if ((uintptr_t)i & 0xfff) exit(6); // ensure aligned
    void* j = malloc(1);
    if ((uintptr_t)i != (uintptr_t)j + 0x1004) exit(6); // make sure we jumped a full alignment (0x1000 plus our misalignment 4)
    free(j);
    free(i);
    free(g);
    free(h);

    // test errno on a bad alignment
    errno = EAGAIN;
    void* k = aligned_alloc(7, 16);
    if (k != NULL) exit(7);
    if (errno != EINVAL) exit(8);

    // test when the allocated pointer is aligned naturally
    void* l = aligned_alloc(4, 17);
    if (((intptr_t)l & 3) != 0) exit(9);
    free(l);

}

static void test_posix_memalign(void) {
    void* p = (void*)0xDEADC0DEu;
    errno = EAGAIN;

    // make a bad call to posix_memalign()
    int ret = posix_memalign(&p, 7, 13);

    // check error handling
    if (ret != EINVAL) exit(9); // error is returned
    if (errno != EAGAIN) exit(10); // errno is not modified
    if ((uintptr_t)p != 0xDEADC0DEu) exit(11); // pointer is not modified

}

int main(void) {
    test_aligned_alloc();
    test_posix_memalign();
}
