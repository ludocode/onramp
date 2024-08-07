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

#include <stdlib.h>

#include <errno.h>
#include <limits.h>
#include <string.h>

extern int* __process_info_table;

            //#include <stdio.h>
            //void putd(int number);

#ifdef __linux__
int printf(const char* restrict format, ...); // TODO debugging only
#endif

/*
 * This is Onramp's omC implementation of malloc().
 *
 * It is similar to (but not exactly the same as) the example storage allocator
 * in section 8.7 of The C Programming Language Second Edition by Kernighan and
 * Ritchie.
 *
 * Each allocation is preceded by a single word header indicating its size.
 * Free allocations are stored in an intrusive singly-linked list sorted by
 * address. malloc() performs a linear best-fit search of the free list
 * splitting allocations as needed. free() performs a linear search of the free
 * list to find the insertion point and to coalesce adjacent free allocations.
 *
 * omC doesn't support structs or arrays so everything is done with bare int
 * pointers. Sizes are in ints, not in bytes. The free list links are stored
 * intrusively instead of as part of the header.
 *
 * This code assumes pointers are 32 bits, as they are in Onramp. It can be
 * compiled and tested on ILP32 Linux. It could probably be modified to work on
 * any platform with some slightly more clever pointer arithmetic but I haven't
 * bothered.
 */

// TODO this allocator does work but it needs a lot of cleanup, either that or
// we should just delete it. If we can modify the libc/0 allocator to use a
// free list, even if it doesn't coalesce free blocks, that would be good
// enough to bootstrap the libc/2 allocator and maybe we could get rid of this
// stage of the libc entirely.

#ifdef __GNUC__
// TODO for now we disable these warnings. The code only works on 32-bit
// platforms. See above, not sure if it's worth fixing yet.
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#endif

static int* free_list;

#ifdef __onramp_libc1_test
// unit tests use these, we don't need to store them otherwise
static char* heap_start;
static char* heap_end;
#endif

void __malloc_init(void) {
    #ifndef __onramp_libc1_test
    char* heap_start;
    char* heap_end;
    #endif

    #ifdef __onramp__
        // The heap start is the program break aligned to 32 bits.
        heap_start = *(__process_info_table + 1);

        // The heap end is the bottom of the stack. For now we reserve 32kB for
        // the stack. We just subtract it from the address of a local variable,
        // close enough.
        char* dummy;
        heap_end = ((char*)&dummy - 32768);
    #endif

    #ifndef __onramp__
        // When running unit tests on other platforms we just use a fixed heap.
        static int heap[131072];
        //printf("heap is at %p\n",heap);
        heap_start = (char*)heap;
        heap_end = (char*)heap + sizeof(heap);
    #endif

    // align start and end pointers
    heap_start = (char*)((int)(heap_start + 3) & (~3));
    heap_end = (char*)((int)heap_end & (~3));

    int heap_size = ((int)(heap_end - heap_start) >> 2);
    if (heap_size <= 2) {
        // no heap space, just return. the free list will be empty so malloc()
        // will fail with out-of-memory.
        return;
    }

    free_list = ((int*)heap_start + 1);
    *(free_list - 1) = (heap_size - 1);
    *free_list = 0;
    //printf("free list is at %p\n",&free_list);
}

void* malloc(size_t usize) {
    if (usize == 0) {
        usize = 1;
    }
//puts("--------");
//fputs("malloc ", stdout);
//putd(usize);
//fputc('\n', stdout);
    if (usize > (INT_MAX >> 2)) {
        return 0;
    }
    int size = (int)usize;

    // Convert to a number of ints rounded up.
    size = ((size + 3) >> 2);
    //printf("malloc() size %zi\n",size);

//fputs("malloc size in words: ", stdout);
//putd(size);
//fputc('\n', stdout);

    // Scan the list for the smallest allocation that fits. (In case of ties,
    // use the first one.)
    int* best = 0;            // the best allocation
    int best_size = INT_MAX;  // size of the best allocation
    int* best_link = 0;       // linked list pointer to the best allocation
    int* current_link = (int*)&free_list;
    while (*current_link != 0) {
        int* alloc = (int*)*current_link;
        int alloc_size = *(alloc - 1);

        if (alloc_size == size) {
            // We've found an exact match. Pop this allocation and return it.
//fputs("exact match: ", stdout);
//putd((int)alloc);
//fputc('\n', stdout);
            *current_link = *alloc;
            return alloc;
        }

        if ((alloc_size > size) & (alloc_size < best_size)) {
            // We've found an allocation large enough.
            best_link = current_link;
            best = alloc;
            best_size = alloc_size;
            // Keep searching for a better fit.
        }

        current_link = alloc;
    }

    // If none fit, we're out of memory.
    if (best == 0) {
        errno = ENOMEM;
        return NULL;
    }

//fputs("found: ", stdout);
//putd((int)best);
//fputc('\n', stdout);

    // We've found a free allocation but it's too large. We need at least 8
    // extra bytes to split it.
    if (best_size < (size + 8)) {
        // Can't split. Just pop and return.
//puts("can't split, returning");
        *best_link = *best;
        return best;
    }

    // Split it...
    int* remainder = ((best + size) + 1);
    //printf("best %p remainder %p\n",best,remainder);
    *(best - 1) = size;
    *(remainder - 1) = ((best_size - size) - 1);

    // ...and insert the remainder in the list.
    *best_link = (int)remainder;
    *remainder = *best;
//puts("split. returning");
    return best;
}

void free(void* v) {
    if (v == 0) {
        return;
    }

    int* ptr = (int*)v;
    int ptr_size = *(ptr - 1);
    //printf("free() %p size %zi\n",ptr,ptr_size);

    // Scan the free list to figure out where this allocation goes.
    int* current_link = (int*)&free_list;
    int* previous = 0;
    int* next = (int*)*current_link;
    while ((next != 0) & (next < ptr)) {
        previous = next;
        current_link = next;
        next = (int*)*next;
    }

    //printf("found. next %p previous %p\n",next,previous);

    // Try to merge with the next allocation
    if ((next != 0) & (((ptr + ptr_size) + 1) == next)) {
        //printf("merging right\n");
        int next_size = *(next - 1);
        ptr_size = (ptr_size + (next_size + 1));
        *(ptr - 1) = ptr_size;
        next = (int*)*next;
    }

    // Try to merge with the previous allocation.
    if (previous != 0) {
        //printf("merging left\n");
        int previous_size = *(previous - 1);
        if (((previous + previous_size) + 1) == ptr) {
            *(previous - 1) = ((ptr_size + previous_size) + 1);
            *previous = (int)next;
            return;
        }
    }

    // Link us into the list.
    *current_link = (int)ptr;
    *ptr = (int)next;
}

void* realloc(void* old_ptr, size_t new_size) {
    // TODO for now we don't bother to try to grow.
    void* new_ptr = malloc(new_size);
    if ((old_ptr != NULL) & (new_ptr != NULL)) {
        size_t old_size = ((size_t)*((int*)old_ptr - 1) << 2);
        if (old_size > new_size) {
            old_size = new_size;
        }
        memcpy(new_ptr, old_ptr, old_size);
        free(old_ptr);
    }
    return new_ptr;
}

void* __malloc_largest_unused_region(size_t* out_size) {
    int* ret = 0;
    int ret_size = 0;
    int* current_link = (int*)&free_list;
    while (*current_link != 0) {
        int* alloc = (int*)*current_link;
        int alloc_size = *(alloc - 1);
        if (alloc_size >= ret_size) {
            ret = alloc;
            ret_size = alloc_size;
        }
        current_link = alloc;
    }

    // TODO check that we actually have a region

    // skip intrusive linked list pointer (since we aren't actually allocating this)
    ret = (ret + 1);
    ret_size = (ret_size - 1);

    *out_size = (ret_size << 2); // convert to bytes
    return ret;
}
