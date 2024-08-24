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

#include "internal.h"

#include <stdbool.h>

// TODO disabled until cpp/2 is working, we need function-like macros
#ifndef __onramp__

/*
 * qsort() and qsort_r()
 *
 * The implementation isn't actually quicksort. Instead we use a shellsort
 * with the Ciura gap sequence. It's simpler and the compiled code is smaller,
 * and it has great performance for nearly-sorted arrays. The downside is it's
 * not quite O(nlogn), though the standard doesn't actually promise that so
 * we're fine.
 *
 * This implementation is adapted from Ghost, which was originally adapted from
 * Pottery.
 *     https://github.com/ludocode/ghost/tree/develop/include/ghost/impl/algorithm/qsort
 *     https://github.com/ludocode/pottery/tree/develop/include/pottery/shell_sort
 */

typedef struct qsort_state_t {
    size_t element_size;
    bool has_context;
    union {
        int (*without_context)(const void* left, const void* right);
        int (*with_context)(const void* left, const void* right, void* user_context);
    } user_compare;
    void* user_context;
} qsort_state_t;

#define qsort_compare(state, left, right) \
    (((state)->has_context) ? \
        (state)->user_compare.without_context(left, right) : \
        (state)->user_compare.with_context(left, right, state->user_context))

#define qsort_select(state, base, v_index) \
    ((char*)(base) + (v_index) * (state)->element_size)

#define qsort_compare_less(state, left, right) \
    (0 > qsort_compare(state, left, right))

static void qsort_swap_restrict(qsort_state_t* state, void* vleft, void* vright) {
    char* left = (char*)vleft;
    char* right = (char*)vright;
    char* end = right + state->element_size;
    while (right != end) {
        char temp = *left;
        *left++ = *right;
        *right++ = temp;
    }
}

// We use the Ciura gap sequence extended by *2.25 (A102549).
static size_t qsort_gaps[] = {
    // The original Ciura numbers
    1u, 4u, 10u, 23u, 57u, 132u, 301u, 701u,
    // Extension multiplying by 2.25 (9/4)
    1577u, 3548u, 7983u, 17961u, 40412u, 90927u, 204585u, 460316u,
    1035711u, 2330349u, 5243285u, 11797391u, 26544129u, 59724290u,
    134379652u, 302354217u, 680296988u, 1530668223u, 3444003501u
};
#define QSORT_GAPS_MAX (sizeof(qsort_gaps) / sizeof(*qsort_gaps))

static void qsort_impl(qsort_state_t* state, void* base, size_t count) {

    /* Figure out where to start */
    size_t gap_index = 0;
    while (qsort_gaps[gap_index] > count / 2 && gap_index < QSORT_GAPS_MAX - 1)
        ++gap_index;

    /* Perform successive insertion sorts based on gap sequence */
    for (;;) {
        size_t gap = qsort_gaps[gap_index];
        size_t i;
        for (i = gap; i < count; ++i) {
            size_t j = i;
            while (j >= gap && qsort_compare_less(state,
                            qsort_select(state, base, j),
                            qsort_select(state, base, j - gap)))
            {
                qsort_swap_restrict(state,
                        qsort_select(state, base, j),
                        qsort_select(state, base, j - gap));
                j -= gap;
            }
        }
        if (gap_index == 0)
            break;
        --gap_index;
    }
}

void qsort(void* first, size_t count, size_t element_size,
        int (*user_compare)(const void* left, const void* right))
{
    qsort_state_t state;
    state.element_size = element_size;
    state.has_context = 0;
    state.user_compare.without_context = user_compare;
    qsort_impl(&state, first, count);
}

void qsort_r(void* first, size_t count, size_t element_size,
        int (*user_compare)(const void* left, const void* right, void* user_context),
        void* user_context)
{
    qsort_state_t state;
    state.element_size = element_size;
    state.has_context = 1;
    state.user_compare.with_context = user_compare;
    state.user_context = user_context;
    qsort_impl(&state, first, count);
}

#endif
