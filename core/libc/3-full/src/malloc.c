/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023-2025 Fraser Heavy Software
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

#include "internal.h"

#include <__onramp/__pit.h>
#include <__onramp/__syscalls.h>
#include <malloc.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdbit.h>

/**
 * This is Onramp's full implementation of malloc().
 *
 * It is a bog-standard boundary tag allocator, simple as possible without
 * being completely terrible. Allocations are rounded up to a multiple of the
 * minimum size (8). Allocations are coalesced on free and stored in unsorted
 * free lists for each power-of-two size class.
 *
 * Since the Onramp VM doesn't support multithreading, we don't need to worry
 * about any locks, contention, thread caches, etc. We can also assume pointers
 * are 32 bits.
 */

// NOTE: There is quite a bit of printf() code here that can be uncommented to
// debug this. Note that malloc() is initialized before io, and __io_init()
// needs malloc() so these print calls will crash on startup. Some hacks are
// needed to use them. See io.c and system.c .

// TODO we don't build on 64-bit systems currently so we can't build natively
// for testing. For now we just build on onramp only.
#ifdef __onramp__

/*
 * The allocator is made extra simple because we don't have to worry about
 * getting extra memory with sbrk() or mmap() and we aren't deferring larger
 * allocations to mmap(). The heap is just one big contiguous chunk of memory.
 */
char* __heap_start;
char* __heap_end;

/**
 * A word at the bottom of the stack with a fixed value. If this value is ever
 * overwritten, it means a stack overflow has occurred.
 *
 * We only check the canary on malloc()-like functions and free(). This won't
 * always (or even often) detect a stack overflow but it helps.
 */
static int* stack_canary;
#define STACK_CANARY 0x5A4B3C2D

static void detect_stack_overflow(void) {
    #ifdef __onramp__
    if (*stack_canary != STACK_CANARY) {
        __fatal("ERROR: A stack overflow occurred.\n");
    }
    #endif
}

/*
 * The boundary tags indicate the size of the allocation and whether it is in
 * use.
 *
 * The lowest bit of the boundary tag is 1 if the allocation is in use and 0
 * otherwise.
 *
 * The higher bits are the size.
 *
 * The boundary tags before and after an allocation must match (including the
 * in-use bit.)
 */
#define HEADER_TAG(ptr)       (*((size_t*)(ptr) - 1))
#define FOOTER_TAG(ptr, size) (*((size_t*)((char*)(ptr) + (size))))
#define PREVIOUS_TAG(ptr)     (*((size_t*)(ptr) - 2))
#define NEXT_TAG(ptr, size)   (*((size_t*)((char*)(ptr) + (size)) + 1))

/**
 * This is the overhead from the two tags.
 */
#define TAGS_SIZE (2 * sizeof(size_t))

/**
 * Free allocations are stored in intrusive doubly-linked lists.
 *
 * If a free allocation's previous is NULL, it's at the start of the list, so
 * it is pointed to by the free_list[] entry of its size class.
 */
typedef struct free_alloc_t {
    struct free_alloc_t* previous;
    struct free_alloc_t* next;
} free_alloc_t;

/**
 * The need to store pointers makes the minimum allocation size 8.
 */
#define MINIMUM_SIZE (sizeof(free_alloc_t))

/**
 * The minimum alignment on Onramp is 4. There are no SIMD instructions or
 * anything else in the system that could require larger alignment (however
 * programs can still request larger alignment for their own purposes with
 * aligned_alloc().)
 */
#define MINIMUM_ALIGNMENT (sizeof(size_t))

#define FREE_LIST_COUNT 32

/**
 * We have a separate free list for each power of two. Unused allocations are
 * stored in the free list indexed by their highest set bit. (This means all
 * power-of-two allocations are the *smallest* possible allocations in their
 * size class, so we minimize scanning in cases where a power of two is
 * requested.)
 *
 * The free lists are unsorted. This likely causes additional fragmentation. It
 * is also possible that a non-power-of-two allocation could cause significant
 * churn in search of a free allocation that fits. These are deemed acceptable
 * tradeoffs to maintain simplicity, to avoid having to implement a red-black
 * tree or other mechanism to sort them.
 *
 * Note that the lowest lists are unusable since we have a minimum allocation
 * size of two pointers. We don't bother offsetting size classes to fix this.
 *
 * This is an array of free lists. We allocate it manually because otherwise
 * it would end up as bss which requires malloc().
 */
static free_alloc_t** free_list;

/**
 * Simple function to get the position of the highest set bit in a number. This
 * is used to determine which free list an allocation goes in.
 *
 * TODO use stdc_leading_zeros()
 */
static int highest_bit(unsigned v) {
    int r = 0;

    // This is an unrolling of:
    //     https://graphics.stanford.edu/~seander/bithacks.html#IntegerLog

    if (v & 0xFFFF0000) {
        v >>= 16;
        r |= 16;
    }
    if (v & 0xFF00) {
        v >>= 8;
        r |= 8;
    }
    if (v & 0xF0) {
        v >>= 4;
        r |= 4;
    }
    if (v & 0xC) {
        v >>= 2;
        r |= 2;
    }
    if (v & 0x2) {
        v >>= 1;
        r |= 1;
    }

    return r;
}

// TODO this belongs in <stdbit.h> and should be a _Generic macro
// (and also we should have a better implementation)
static int stdc_has_single_bit(size_t x) {
    return x != 0 && (x & ~(1 << highest_bit(x))) == 0;
}

static void add_to_free_list(free_alloc_t* alloc, size_t size) {
    int size_class = highest_bit(size);
    //printf("  adding to free list: 0x%p size %zi class %i\n", alloc, size, size_class);
    if (free_list[size_class] != NULL) {
        libc_assert(free_list[size_class]->previous == NULL);
        free_list[size_class]->previous = alloc;
    }
    alloc->next = free_list[size_class];
    alloc->previous = NULL;
    free_list[size_class] = alloc;
}

static void remove_from_free_list(free_alloc_t* alloc, size_t size) {
    //printf("  removing from free list: 0x%p %zi\n", alloc, size);
    if (alloc->next != NULL) {
        alloc->next->previous = alloc->previous;
    }
    if (alloc->previous != NULL) {
        alloc->previous->next = alloc->next;
    } else {
        int size_class = highest_bit(size);
        libc_assert(free_list[size_class] == alloc);
        free_list[size_class] = alloc->next;
    }
}

#if defined(__linux__) && defined(__i386__)
// This is the Linux interface for SYS_brk, not the classic UNIX interface
// provided by libcs. It sets the break to the given address, returning it if
// successful and returning the old break otherwise. Pass null to query the
// current break. It's only used to initialize the heap.
static void* linuxbrk(void* addr) {
    void* ret;
    __asm__(
            "movl $45, %%eax\n"  // 45 == SYS_brk on i386
            "movl %[addr], %%ebx\n"
            "int $0x80\n"
            "movl %%eax, %[ret]\n"
            : // outputs
            [ret] "=r" (ret)
            : // inputs
            [addr] "rm" (addr)
            : // registers clobbered
            "eax",
            "ebx"
    );
    return ret;
}
#endif

#ifdef ONRAMP_LIBC_OVERLAY
__attribute__((constructor))
#endif
void __malloc_init(void) {
    #ifdef __onramp__

        // The heap start is the program break aligned to 32 bits. (It's
        // aligned below.)
        __heap_start = __process_info_table[__ONRAMP_PIT_BREAK];

        // The heap end is the bottom of the stack. For now we reserve 128 kB
        // for the stack which is the same as musl.
        // TODO use the real stack base, for now we just subtract it from the
        // address of a local variable.
        char* dummy;
        __heap_end = (char*)&dummy - 128 * 1024;
        //printf("__heap_start 0x%p __heap_end 0x%p dummy stack var 0x%p\n", __heap_start, __heap_end, &dummy);

    #elif defined(__linux__)
        // On Linux we allocate a big chunk to the heap.
        __heap_start = (char*)linuxbrk(NULL);
        __heap_end = (char*)linuxbrk(__heap_start + 1024*1024*512);
        //__heap_end = (char*)linuxbrk(__heap_start + 4096);
    #else
        #error "Platform not supported."
    #endif

    // align start and end pointers
    __heap_start = (char*)((int)(__heap_start + 3) & (~3));
    __heap_end = (char*)((int)__heap_end & (~3));

    if (__heap_end < __heap_start || (uintptr_t)(__heap_end - __heap_start) < (uintptr_t)(64 * sizeof(size_t))) {
        // Not enough heap to do anything useful. We will need an allocator to
        // setup stdio so we can't do anything.
        // TODO we need to use the alloc syscall if available to grow the heap.
        abort();
    }

    // allocate the free lists
    free_list = (free_alloc_t**)__heap_start;
    __heap_start += FREE_LIST_COUNT * sizeof(size_t);
    for (size_t i = 0; i < FREE_LIST_COUNT; ++i) {
        free_list[i] = NULL;
    }

    #ifdef __onramp__
    // Place a canary at the bottom of the stack to detect stack overflow.
    stack_canary = (int*)__heap_end;
    *stack_canary = STACK_CANARY;
    #endif

    // Place sentinel tags at either end of the heap. This eliminates special
    // cases in our merging code: the ends of the heap will be treated as
    // allocations in use.
    *(size_t*)__heap_start = 1;
    __heap_start += sizeof(size_t);
    __heap_end -= sizeof(size_t);
    *(size_t*)__heap_end = 1;

    // The rest of the heap starts out as one big free allocation.
    size_t size = __heap_end - __heap_start - 2 * sizeof(size_t);
    *(size_t*)__heap_start = size;     // header tag
    *((size_t*)__heap_end - 1) = size; // footer tag
    free_alloc_t* alloc = (free_alloc_t*)(__heap_start + sizeof(size_t));
    alloc->previous = NULL;
    alloc->next = NULL;
    free_list[highest_bit(size)] = alloc;
}

// TODO try_grow and try_shrink are always used together, we should just merge them into try_resize()

/*
 * Try to merge this (in use) allocation with the next one if it's free.
 */
static void try_grow(void* ptr, size_t size) {
    assert(HEADER_TAG(ptr) & 1); // ensure in use
    assert((HEADER_TAG(ptr) & ~(size_t)1) == size); // ensure given size is correct

    size_t next_tag = NEXT_TAG(ptr, size);
    if (next_tag & 1) {
        //printf("  try grow 0x%p can't grow\n",ptr);
        // The next allocation is in use. We can't use it.
        return;
    }

    // The next allocation is free. We can merge.
    free_alloc_t* next = (free_alloc_t*)((char*)ptr + size + 2 * sizeof(size_t));
    libc_assert(next_tag == FOOTER_TAG(next, next_tag));
    remove_from_free_list(next, next_tag);

    //printf("  try grow 0x%p size %zi merging with 0x%p new size %zi\n",ptr, size, (void*)next, size + next_tag + TAGS_SIZE);
    size = size + next_tag + TAGS_SIZE;
    HEADER_TAG(ptr) = size | 1;
    FOOTER_TAG(ptr, size) = size | 1;
}

/*
 * Try to split ptr down to the given size, putting any remainder back in the
 * free lists.
 */
static void try_shrink(void* ptr, size_t current_size, size_t requested_size) {

    // If the next allocation is free, we have to first merge it into this one
    // before we can split, otherwise we'll have two adjacent free allocations.
    try_grow(ptr, current_size);

    size_t tag = HEADER_TAG(ptr);
    size_t size = tag & ~(size_t)1;
    //printf("  try shrink 0x%p of size %zi to %zi\n",ptr, size, requested_size);
    libc_assert(tag & 1); // the allocation must be in used
    libc_assert(tag == FOOTER_TAG(ptr, size));
    libc_assert(requested_size <= size); // the requested size must be smaller

    if (size - requested_size >= TAGS_SIZE + MINIMUM_SIZE) {
        size_t other_size = size - requested_size - TAGS_SIZE;

        // Shrink this allocation
        HEADER_TAG(ptr) = requested_size | 1;
        FOOTER_TAG(ptr, requested_size) = requested_size | 1;

        // Create the rest and put it back
        free_alloc_t* other = (free_alloc_t*)((char*)ptr + requested_size + 2 * sizeof(size_t));
        HEADER_TAG(other) = other_size;
        FOOTER_TAG(other, other_size) = other_size;
        add_to_free_list(other, other_size);
    }
}

// Rounds up the given size to the minimum size and natural alignment of
// allocations.
//
// This is malloc_good_size() on macOS and nallocx() (without flags) on
// FreeBSD. We could expose it as these at some point.
static size_t round_size(size_t requested_size) {

    // There's a minimum size to fit our free list pointers. (We also always
    // provide a unique address even if 0 bytes are requested.)
    if (requested_size < MINIMUM_SIZE) {
        requested_size = MINIMUM_SIZE;
    }

    // Allocations must remain aligned.
    requested_size = (requested_size + (MINIMUM_ALIGNMENT - 1)) & ~(size_t)(MINIMUM_ALIGNMENT - 1);

    return requested_size;
}

void* malloc(size_t requested_size) {
    detect_stack_overflow();

    //__debugprint_su("malloc requested", requested_size);
    requested_size = round_size(requested_size);
    int size_class = highest_bit(requested_size);
    //__debugprint_su("  rounded to", requested_size);
    //__debugprint_su("size class", size_class);

    // First search the free list of the requested size class to see if any
    // existing free allocations will fit.
    free_alloc_t* found = NULL;
    for (free_alloc_t* alloc = free_list[size_class]; alloc != NULL; alloc = alloc->next) {
        size_t alloc_size = HEADER_TAG(alloc);
        libc_assert((alloc_size & 1) == 0); // allocation must be unused
        libc_assert(alloc_size == FOOTER_TAG(alloc, alloc_size)); // tags must match
        if (requested_size > alloc_size)
            continue;
        //__debugprint_su("size class", size_class);
        //printf("  found in matching size class %i\n",size_class);
        remove_from_free_list(alloc, alloc_size);
        found = alloc;
        break;
    }

    // Otherwise search higher size classes. No need to scan the lists for one
    // that fits; it is guaranteed to fit so we just find the first one that
    // isn't null.
    if (found == NULL) {
        while (++size_class < FREE_LIST_COUNT) {
            if (free_list[size_class] == NULL)
                continue;
            //printf("  found in higher size class %i\n",size_class);
            found = free_list[size_class];
            remove_from_free_list(found, HEADER_TAG(found));
            break;
        }
    }

    if (!found) {
        errno = ENOMEM;
        return NULL;
    }

    // Make sure what we found is valid
    size_t found_size = HEADER_TAG(found);
    //printf("  found 0x%p size %zi\n",found,found_size);
    libc_assert((found_size & 1) == 0); // allocation must be unused
    libc_assert(found_size >= requested_size);
    libc_assert(found_size == FOOTER_TAG(found, found_size)); // tags must match

    // Mark it as used
    HEADER_TAG(found) |= 1;
    FOOTER_TAG(found, found_size) |= 1;

    // See if there is enough space left over to split it
    try_shrink(found, found_size, requested_size);

    // Done
    //printf("  malloc() returning 0x%p\n",found);
    return found;
}

void free(void* ptr) {
    if (ptr == NULL)
        return;

    detect_stack_overflow();

    size_t tag = HEADER_TAG(ptr);
    size_t size = tag & ~(size_t)1;
    //printf("free() freeing 0x%p size %zi\n",ptr, size);

    // Some checks against memory corruption:
    // The tag must indicate that this pointer is in use.
    libc_assert(tag & 1);
    // Boundary tags must match.
    libc_assert(tag == FOOTER_TAG(ptr, size));

    // Mark the block as unused
    HEADER_TAG(ptr) = size;
    FOOTER_TAG(ptr, size) = size;

    // Try to merge with preceding block
    if ((char*)ptr - sizeof(size_t) != __heap_start) {
        size_t previous_size = PREVIOUS_TAG(ptr);
        if ((previous_size & 1) == 0) {
            free_alloc_t* previous = (free_alloc_t*)((char*)ptr - 2 * sizeof(size_t) - previous_size);
            //printf("  merging with previous block 0x%p size %zi\n", previous, previous_size);
            libc_assert(previous_size == HEADER_TAG(previous));
            remove_from_free_list(previous, previous_size);

            ptr = previous;
            size = size + previous_size + TAGS_SIZE;
            HEADER_TAG(ptr) = size;
            FOOTER_TAG(ptr, size) = size;
        }
    }

    // Try to merge with following block
    if ((char*)ptr + size + sizeof(size_t) != __heap_end) {
        size_t next_size = NEXT_TAG(ptr, size);
        if ((next_size & 1) == 0) {
            free_alloc_t* next = (free_alloc_t*)((char*)ptr + size + 2 * sizeof(size_t));
            //printf("  merging with next block 0x%p size %zi\n", next, next_size);
            libc_assert(next_size == FOOTER_TAG(next, next_size));
            remove_from_free_list(next, next_size);

            size = size + next_size + TAGS_SIZE;
            HEADER_TAG(ptr) = size;
            FOOTER_TAG(ptr, size) = size;
        }
    }

    // Place it in the appropriate free list
    add_to_free_list((free_alloc_t*)ptr, size);
}

size_t malloc_size(void* ptr) {
    return *((size_t*)ptr - 1);
}

void* realloc(void* ptr, size_t new_size) {
    //printf("realloc() resizing 0x%p to size %zi\n", ptr, new_size);

    // realloc(NULL, size) is equivalent to malloc(size).
    if (ptr == NULL) {
        //printf("  ptr is null; forwarding to malloc()\n");
        return malloc(new_size);
    }

    // realloc() of a non-null pointer with a size of zero is undefined
    // behaviour.
    if (new_size == 0) {
        //printf("  resize of non-null to 0 is undefined\n");
        abort();
    }

    detect_stack_overflow();

    new_size = round_size(new_size);
    size_t tag = HEADER_TAG(ptr);
    size_t size = tag & ~(size_t)1;

    // The same corruption checks as in free() above
    libc_assert(tag & 1);
    libc_assert(tag == FOOTER_TAG(ptr, size));

    // If the requested size isn't more than the current size, just try to
    // shrink it. We don't do anything else.
    if (new_size <= size) {
        try_shrink(ptr, size, new_size);
        return ptr;
    }

    // See if we can merge this block with the next one
    try_grow(ptr, size);
    size_t grow_size = malloc_usable_size(ptr);

    // If we're now large enough, we can shrink back down to the new size and
    // we're done.
    if (grow_size >= new_size) {
        try_shrink(ptr, grow_size, new_size);
        return ptr;
    }

#if 0
    // See if the next block can be merged into this one (and if it would be
    // large enough to satisfy the requested allocation.) We do this even if
    // we're shrinking because we need to merge the slack.
    if ((char*)ptr + size + sizeof(size_t) != __heap_end) {
        size_t next_size = NEXT_TAG(ptr, size);
        if ((next_size & 1) == 0 && size + next_size >= new_size) {

            // We can merge.
            free_alloc_t* next = (free_alloc_t*)((char*)ptr + size + 2 * sizeof(size_t));
            libc_assert(next_size == FOOTER_TAG(next, next_size));
            remove_from_free_list(next, next_size);

            size = size + next_size + TAGS_SIZE;
            HEADER_TAG(ptr) = size | 1;
            FOOTER_TAG(ptr, size) = size | 1;

            // If there's any left over, put it back in.
            try_shrink(ptr, size, new_size);
            return ptr;
        }
    }
#endif

    // Otherwise we have to move.
    void* new_ptr = malloc(new_size);
    if (new_ptr == NULL) {
        // malloc() already set errno
        return NULL;
    }
    memcpy(new_ptr, ptr, size);
    free(ptr);
    return new_ptr;
}

void* aligned_alloc(size_t alignment, size_t requested_size) {
    //printf("aligned_alloc requested alignment %zi size %zi\n", alignment, requested_size);

    // aligned_alloc() on C11 requires that the size be a multiple of the
    // alignment. This requirement was relaxed in C17 so we don't check.
    requested_size = round_size(requested_size);

    // The only requirement for alignment is that it is an alignment supported
    // by the implementation. This implementation supports any power of two.
    if (!stdc_has_single_bit(alignment)) {
        //printf("  bad alignment!\n");
        errno = EINVAL;
        return NULL;
    }

    // We allocate a chunk that is the size plus the alignment, this way we can
    // align the pointer within it and return the excess on both sides to the
    // heap. We also add the overhead of an empty allocation (tags plus
    // pointers) to ensure that we can make enough space on the left to store
    // metadata. This can require a fair bit of extra memory for large
    // alignments but the excess is immediately returned to the heap.
    size_t alloc_size = requested_size + alignment + TAGS_SIZE + MINIMUM_SIZE;
    //printf("  rounded size to %zi, allocating %zi\n",requested_size, alloc_size);
    void* p = malloc(alloc_size);
    if (!p) {
        // malloc() set errno
        return NULL;
    }

    // Check alignment
    libc_assert(stdc_has_single_bit(alignment));
    uintptr_t mask = (uintptr_t)(alignment - 1);
    uintptr_t pu = (uintptr_t)p;
    if (pu & ~mask == 0) {
        // It's already aligned. Just cut right and return.
        //printf("  already aligned. shrinking\n");
        size_t current_size = HEADER_TAG(p) & ~(size_t)1;
        try_shrink(p, current_size, requested_size);
        //printf("  final allocation size is %zi\n", HEADER_TAG(p)&~(size_t)1);
        //printf("  returning 0x%p\n", p);
        return p;
    }

    // Align the pointer
    uintptr_t qu = (pu + mask) & ~mask;
    if (qu - pu < TAGS_SIZE + MINIMUM_SIZE) {
        // Not quite aligned on the left, but not enough room for metadata. We
        // jump to the next multiple of alignment, or at least enough that we
        // can fit a left allocation. (This is why we needed to reserve extra
        // space for metadata above.)
        //printf("  not enough to cut left; jumping to next alignment\n");
        if (alignment < MINIMUM_SIZE + TAGS_SIZE) {
            qu += MINIMUM_SIZE + TAGS_SIZE;
        } else {
            qu += alignment;
        }
    }
    void* q = (void*)qu;
    //printf("  aligned to 0x%p\n", q);
    libc_assert(qu - pu + requested_size <= alloc_size);

    // Return excess left to the heap
    size_t left_size = qu - pu - TAGS_SIZE;
    //printf("  cutting left allocation of %zi\n", left_size);
    libc_assert(left_size >= MINIMUM_SIZE);
    HEADER_TAG(p) = left_size;
    FOOTER_TAG(p, left_size) = left_size;
    add_to_free_list(p, left_size);
    size_t new_size = alloc_size - (qu - pu);
    //printf("  new allocation size is %zi\n", new_size);
    libc_assert(new_size >= requested_size);
    HEADER_TAG(q) = new_size | 1;
    FOOTER_TAG(q, new_size) = new_size | 1;

    // Return excess right to the heap
    //printf("  shrinking\n");
    try_shrink(q, new_size, requested_size);
    //printf("  final allocation size is %zi\n", HEADER_TAG(q)&~(size_t)1);
    //printf("  returning 0x%p\n", q);
    return q;
}

int posix_memalign(void** out_ptr, size_t alignment, size_t size) {

    // posix_memalign() requires that the alignment is at least sizeof(void*)
    // and is a power of two. (aligned_alloc() checks for power of two.)
    if (alignment < sizeof(void*))
        return EINVAL;

    // posix_memalign() must not modify the output pointer or set errno on
    // failure. We preserve errno before and restore it after the call.
    int prev_err = errno;
    void* p = aligned_alloc(alignment, size);
    if (!p) {
        int err = errno;
        errno = prev_err;
        return err;
    }
    *out_ptr = p;
    return 0;
}

/**
 * Returns the largest contiguous chunk of memory available.
 *
 * This is used by __onramp_spawn() to provide as much free memory as possible
 * to the child process.
 *
 * This does not actually allocate it; it just returns the usable area. It is
 * not safe to call any allocation functions while the area is in use.
 */
void* __malloc_unused_region(size_t* out_size) {
    detect_stack_overflow();

    free_alloc_t* p;
    size_t size = 0;

    // Find the highest size class with an allocation in it
    for (size_t size_class = FREE_LIST_COUNT; size_class-- != 0; ) {
        if (free_list[size_class] == NULL)
            continue;

        // Find the largest allocation in this size class (usually there's only
        // one but we check anyway)
        for (free_alloc_t* f = free_list[size_class]; f; f = f->next) {
            assert((HEADER_TAG(f) & 1) == 0); // make sure it's not allocated
            if (HEADER_TAG(f) > size) {
                size = HEADER_TAG(f);
                p = f;
            }
        }
        break;
    }

    // Return the unused portion so we don't corrupt our linked list
    if (size <= sizeof(free_alloc_t)) {
        __fatal("Out of memory.\n");
    }
    *out_size = size - sizeof(free_alloc_t);
    //printf("__malloc_largest_unused_region() returning 0x%p size %zi\n",p+1, *out_size);
    return p + 1;
}

size_t malloc_usable_size(void* p) {
    size_t tag = HEADER_TAG(p);
    assert(tag & 1); // ensure allocation is in use
    size_t size = tag & ~(size_t)1;
    assert(tag == FOOTER_TAG(p, size)); // ensure footer tag matches header tag
    //printf("malloc_usable_size() 0x%p returning %zi\n",p, size);
    return size;
}

void* calloc(size_t count, size_t element_size) {
    // TODO overflow
    size_t size = count * element_size;
    void* ptr = malloc(size);
    if (ptr)
        memset(ptr, 0, size);
    return ptr;
}

void* __memdup(const void* src, size_t count) {
    void* dest = malloc(count);
    if (!dest) {
        // malloc() set errno
        return NULL;
    }
    memcpy(dest, src, count);
    return dest;
}

char* strdup(const char* src) {
    size_t len = strlen(src);
    void* dest = malloc(len + 1);
    if (!dest) {
        // malloc() set errno
        return NULL;
    }
    memcpy(dest, src, len + 1);
    return dest;
}

char* strndup(const char* src, size_t max_len) {
    size_t len = strnlen(src, max_len);
    char* dest = malloc(len + 1);
    if (!dest) {
        // malloc() set errno
        return NULL;
    }
    memcpy(dest, src, len);
    dest[len] = 0;
    return dest;
}

void* __malloc_bss(size_t size) {
    void* p = malloc(size);
    if (!p) {
        abort();
        __fatal("Out of memory allocating a variable with static storage duration (bss).\n");
    }
    return memset(p, 0, size);
}

#endif
