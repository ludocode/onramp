#include <stdlib.h>

#include "internal.h"

#include <stdbool.h>
#include <stdckdint.h>



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



/*
 * rand()
 *
 * An affine congruential PRNG (typically called linear in the literature,
 * though this is a misnomer.)
 */

static unsigned rand_seed = 1;

int rand(void) {
    return rand_r(&rand_seed);
}

void srand(unsigned seed) {
    rand_seed = seed;
}

int rand_r(unsigned* seed) {
    // The first number in the m=2^32 category in Table 4 of the errata to
    // "Tables of Linear Congruential Generators of Different Sizes and Good
    // Lattice Structure" by Steele & Vigna
    //     https://arxiv.org/abs/2001.05304
    //     https://www.iro.umontreal.ca/~lecuyer/myftp/papers/latrules99Errata.pdf
    const unsigned a = 2891336453u;
    const unsigned c = 1u;
    *seed = a * *seed + c; // mod 2^32 is implied
    return *seed & RAND_MAX;
}



/*
 * Other utility functions
 */

void* bsearch(const void* key, const void* vbase,
                size_t element_count, size_t element_size,
                int (*compare)(const void* key, const void* other))
{
    const char* base = (const char*)vbase;
    size_t start = 0;
    size_t end = element_count;

    while (start != end) {
        size_t i = (start + end) >> 1;
        const void* p = base + i * element_size;
        int c = compare(key, p); // key must be the first argument, see C17 7.22.5$2
//printf("-- %i,%i i %i v %i c %i\n", start, end, i, *(int*)p, c);
        if (c == 0)
            return CONST_CAST(void*, p);
        if (c < 0)
            end = i;
        else
            start = i + 1;
    }

    return 0;
}
