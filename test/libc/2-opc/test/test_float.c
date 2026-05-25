// The MIT License (MIT)
// Copyright (c) 2025-2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifdef __onramp__
    #include <__onramp/__arithmetic.h>
#endif
#ifndef __onramp__
    // this test case can be compiled standalone with a native C compiler or onrampcc:
    // gcc -Wall -Wextra -Wpedantic -fsanitize=address -g test/test_float.c -o /tmp/a && /tmp/a
    #include "../../../../core/libc/2-opc/src/float.c"
    #include <time.h>
#endif

            // TODO all of this is disabled on onramp until we get float parsing in
            #ifndef __onramp__

typedef union {
    unsigned u;
    float f;
} floatunion_t;

static float u2f(uint32_t u) {
    floatunion_t xf;
    xf.u = u;
    return xf.f;
}

static uint32_t f2u(float f) {
    floatunion_t xf;
    xf.f = f;
    return xf.u;
}

static int test_sigfpe_count = 0;

static void test_sigfpe(int signo) {
    (void)signo;
    ++test_sigfpe_count;
}

static void test_add_impl(float x, float y, float expected) {
    floatunion_t xf, yf, ef, af;
    xf.f = x;
    yf.f = y;
    ef.f = expected;
    af.u = __float_add(xf.u, yf.u);

    // for nan or zero, we don't need the sign to match x86_64. for nan it
    // doesn't matter (we always return the original sign), and for zero we
    // have separate tests to ensure we match the spec.
    if (isnanf(ef.f) || ef.f == 0) {
        ef.f = fabsf(ef.f);
        af.f = fabsf(af.f);
    }

    if (ef.u != af.u) {
        fprintf(stderr, "Failed to add %.9g + %.9g: expected %.9g, got %.9g\n",
                x, y, ef.f, af.f);
        exit(1);
    }
}

static void test_sub_impl(float x, float y, float expected) {
    floatunion_t xf, yf, ef, af;
    xf.f = x;
    yf.f = y;
    ef.f = expected;
    af.u = __float_sub(xf.u, yf.u);

    // for nan or zero, we don't need the sign to match x86_64. for nan it
    // doesn't matter (we always return the original sign), and for zero we
    // have separate tests to ensure we match the spec.
    if (isnanf(ef.f) || ef.f == 0) {
        ef.f = fabsf(ef.f);
        af.f = fabsf(af.f);
    }

    if (ef.u != af.u) {
        fprintf(stderr, "Failed to sub %.9g - %.9g: expected %.9g, got %.9g\n",
                x, y, ef.f, af.f);
        exit(1);
    }
}

static void test_add(void) {

    // integers less than 1<<24 should be exact
    test_add_impl(1.f, 4.f, 5.f);
    test_add_impl(47.f, 325.f, 372.f);

    // signedness of zero result is specified by IEEE 754 2019
    // x+-x is +0 regardless of sign of input under all rounding except roundToNegative
    test_add_impl(4.f, -4.f, u2f(0));
    test_add_impl(-4.f, 4.f, u2f(0));
    test_add_impl(u2f(0), -u2f(0), u2f(0));
    test_add_impl(-u2f(0), u2f(0), u2f(0));
    // x+x for zero x is x
    test_add_impl(u2f(0), u2f(0), u2f(0));
    test_add_impl(-u2f(0), -u2f(0), -u2f(0));

    // some randomly generated numbers that failed and needed fixing
    test_add_impl(6.10882021e-32, 1.11447999e+32, 1.11447999e+32);
    test_add_impl(1.09704131e+36, 2.66047595e+35, 1.36308884e+36);
    test_add_impl(0.00096412038, 0.00156740146, 0.00253152195);
    test_add_impl(1.99437021e+15, 3.08183242e-21, 1.99437021e+15);
    test_add_impl(1.8693826e-39, 1.0794633e-36, 1.08133265e-36);
    test_add_impl(((floatunion_t){.u=0x25e807}).f, ((floatunion_t){.u=0x185343}).f,
            ((floatunion_t){.u=0x25e807}).f + ((floatunion_t){.u=0x185343}).f); // both subnormal
    test_add_impl(3.00332328e+38, 1.92066509e+38, (float)INFINITY);

    // infinity
    test_add_impl((float)INFINITY, 1.f, (float)INFINITY);
    test_add_impl(1.f, (float)INFINITY, (float)INFINITY);
    test_add_impl((float)INFINITY, ((floatunion_t){.u=1}).f, (float)INFINITY);
    test_add_impl(((floatunion_t){.u=1}).f, (float)INFINITY, (float)INFINITY);
    test_add_impl((float)INFINITY, (float)INFINITY, (float)INFINITY);

            //((floatunion_t){.u=}).f);
    /*
    floatunion_t a, b, result;
    a.f = 1.0f; //  11 1111 1000 0000 0000 0000 0000 0000
    b.f = 4.0f; // 100 0000 1000 0000 0000 0000 0000 0000
    result.u = __float_add(a.u, b.u);
    printf("%f\n", result.f);
    */
}

static void test_sub(void) {

    // integers less than 1<<24 should be exact
    test_sub_impl(4.f, 1.f, 3.f);
    test_sub_impl(400.f, 10.f, 390.f);

    // signedness of zero result is specified by IEEE 754 2019
    // x-x (as x+-x) is +0 regardless of sign of input under all rounding except roundToNegative
    test_sub_impl(4.f, 4.f, u2f(0));
    test_sub_impl(-4.f, -4.f, u2f(0));
    test_sub_impl(u2f(0), u2f(0), u2f(0));
    test_sub_impl(-u2f(0), -u2f(0), u2f(0));
    // x-(-x) for zero x is x
    test_sub_impl(u2f(0), -u2f(0), u2f(0));
    test_sub_impl(-u2f(0), u2f(0), -u2f(0));

    // some randomly generated numbers that failed and needed fixing
    test_sub_impl(u2f(0x69ce4ba), u2f(0x6a0db3c6), u2f(0xea0db3c6));
    test_sub_impl(u2f(0x4a32ce32), u2f(0x56800000), u2f(0xd67fffff));
    test_sub_impl(u2f(0x73f8f94d), u2f(0x73f8f94d), u2f(0));

}

static void test_eq_impl(float x, float y, bool expected) {
    floatunion_t xf, yf;
    xf.f = x;
    yf.f = y;
    bool actual = __float_eq(xf.u, yf.u);
    //printf("0x%x == 0x%x   e %i a %i\n",xf.u,yf.u,expected,actual);

    if (expected != actual) {
        fprintf(stderr, "Failed to compare %.9g == %.9g: expected %i, got %i\n",
                x, y, expected, actual);
        exit(1);
    }
}

static void test_lt_impl(float x, float y, bool expected) {
    floatunion_t xf, yf;
    xf.f = x;
    yf.f = y;
    bool actual = __float_lt(xf.u, yf.u);
    //printf("0x%x < 0x%x   e %i a %i\n",xf.u,yf.u,expected,actual);

    if (expected != actual) {
        fprintf(stderr, "Failed to compare %.9g < %.9g: expected %i, got %i\n",
                x, y, expected, actual);
        exit(1);
    }
}

static void test_lte_impl(float x, float y, bool expected) {
    floatunion_t xf, yf;
    xf.f = x;
    yf.f = y;
    bool actual = __float_lte(xf.u, yf.u);
    //printf("0x%x <= 0x%x   e %i a %i\n",xf.u,yf.u,expected,actual);

    if (expected != actual) {
        fprintf(stderr, "Failed to compare %.9g <= %.9g: expected %i, got %i\n",
                x, y, expected, actual);
        exit(1);
    }
}

static void test_eq(void) {

    // positive and negative zero are equal
    test_eq_impl(0.0, 0.0, true);
    test_eq_impl(0.0, -0.0, true);
    test_eq_impl(-0.0, 0.0, true);
    test_eq_impl(-0.0, -0.0, true);

    // some equal numbers
    test_eq_impl(3.0f, 3.0f, true);
    test_eq_impl(-8.0f, -8.0f, true);

    // nan compares unequal to everything, even itself
    test_eq_impl(u2f(0xFFFFFFFF), u2f(0xFFFFFFFF), false);
    test_eq_impl(u2f(0xFFFFFFFF), u2f(0xFFFFFFFE), false);
    test_eq_impl(INFINITY, u2f(0xFFFFFFFF), false);
    test_eq_impl(-INFINITY, u2f(0xFFFFFFFF), false);

    // infinity should work
    test_eq_impl((float)INFINITY, (float)INFINITY, true);
    test_eq_impl((float)-INFINITY, (float)-INFINITY, true);
    test_eq_impl((float)INFINITY, (float)-INFINITY, false);
    test_eq_impl(0.0f, INFINITY, false);
    test_eq_impl(0.0f, -INFINITY, false);

}

static void test_lt(void) {

    // positive and negative zero are equal
    test_lt_impl(0.0, 0.0, false);
    test_lt_impl(0.0, -0.0, false);
    test_lt_impl(-0.0, 0.0, false);
    test_lt_impl(-0.0, -0.0, false);

    // small number comparisons should work fine
    test_lt_impl(3.0f, 3.0f, false);
    test_lt_impl(3.0f, 4.0f, true);
    test_lt_impl(5.0f, 1.0f, false);
    test_lt_impl(-3.0f, -3.0f, false);
    test_lt_impl(-3.0f, -4.0f, false);
    test_lt_impl(-5.0f, -1.0f, true);

    // differing signs
    test_lt_impl(-5.0f, 1.0f, true);
    test_lt_impl(-1.0f, 5.0f, true);
    test_lt_impl(5.0f, -1.0f, false);
    test_lt_impl(1.0f, -5.0f, false);

    // nan is always false, even when compared to itself
    test_lt_impl(u2f(0xFFFFFFFF), u2f(0xFFFFFFFF), false);
    test_lt_impl(u2f(0xFFFFFFFF), u2f(0xFFFFFFFE), false);
    test_lt_impl(INFINITY, u2f(0xFFFFFFFF), false);
    test_lt_impl(-INFINITY, u2f(0xFFFFFFFF), false);

    // infinity should work
    test_lt_impl((float)INFINITY, (float)INFINITY, false);
    test_lt_impl((float)-INFINITY, (float)-INFINITY, false);
    test_lt_impl((float)INFINITY, (float)-INFINITY, false);
    test_lt_impl((float)-INFINITY, (float)INFINITY, true);
    test_lt_impl(0.0f, INFINITY, true);
    test_lt_impl(0.0f, -INFINITY, false);

}

static void test_lte(void) {

    // positive and negative zero are equal
    test_lte_impl(0.0, 0.0, true);
    test_lte_impl(0.0, -0.0, true);
    test_lte_impl(-0.0, 0.0, true);
    test_lte_impl(-0.0, -0.0, true);

    // small number comparisons should work fine
    test_lte_impl(3.0f, 3.0f, true);
    test_lte_impl(3.0f, 4.0f, true);
    test_lte_impl(5.0f, 1.0f, false);
    test_lte_impl(-3.0f, -3.0f, true);
    test_lte_impl(-3.0f, -4.0f, false);
    test_lte_impl(-5.0f, -1.0f, true);

    // differing signs
    test_lte_impl(-5.0f, 1.0f, true);
    test_lte_impl(-1.0f, 5.0f, true);
    test_lte_impl(5.0f, -1.0f, false);
    test_lte_impl(1.0f, -5.0f, false);

    // nan is always false, even when compared to itself
    test_lte_impl(u2f(0xFFFFFFFF), u2f(0xFFFFFFFF), false);
    test_lte_impl(u2f(0xFFFFFFFF), u2f(0xFFFFFFFE), false);
    test_lte_impl(INFINITY, u2f(0xFFFFFFFF), false);
    test_lte_impl(-INFINITY, u2f(0xFFFFFFFF), false);

    // infinity should work
    test_lte_impl((float)INFINITY, (float)INFINITY, true);
    test_lte_impl((float)-INFINITY, (float)-INFINITY, true);
    test_lte_impl((float)INFINITY, (float)-INFINITY, false);
    test_lte_impl((float)-INFINITY, (float)INFINITY, true);
    test_lte_impl(0.0f, INFINITY, true);
    test_lte_impl(0.0f, -INFINITY, false);
}

// The random float tests don't make sense on Onramp because we need another
// floating point implementation to compare to. When running on x86_64 for
// example, we are comparing it to the x86_64 CPU implementation. Probably this
// should be moved to another file.
#ifndef __onramp__
static uint32_t random_float(void) {
    uint32_t exponent = rand() & FLOAT_EXPONENT_MASK;
    if (exponent == FLOAT_EXPONENT_MASK) {
        exponent = 0u;
    }
    uint32_t significand = rand() & FLOAT_SIGNIFICAND_MASK;

    // TODO for now only generate positive numbers
    //uint32_t sign = rand() & 1u;
    uint32_t sign = 0;

    return (sign << FLOAT_SIGN_SHIFT) | (exponent << FLOAT_EXPONENT_SHIFT) | significand;
}

static void test_add_loop(void) {
    #ifdef __onramp__
    // TODO onramp doesn't have time() yet
    FILE* f = fopen("/dev/urandom", "rb");
    if (f) {
        unsigned x;
        (void)fread(&x, 1, sizeof(x), f);
        (void)fclose(f);
        srand(x);
    }
    #else
    srand(time(NULL));
    #endif

    for (size_t i = 0;; ++i) {
        if (i != 0 && (i % 100000) == 0) printf("testing float addition: %zi...\n",i);

        floatunion_t xf, yf, ef, af;
        xf.u = random_float();
        yf.u = random_float();
        ef.f = xf.f + yf.f;
        //printf("Adding %.9g (0x%x) + %.9g (0x%x), expected %.9g (0x%x)\n", xf.f, xf.u, yf.f, yf.u, ef.f, ef.u);
        af.u = __float_add(xf.u, yf.u);
        //printf("Actual result: %.9g (0x%x)\n", af.f, af.u);

        if (ef.u != af.u) {
            printf("__float_add() FAILED:\n");
            printf("    x: %.9g  0x%08x\n", xf.f, xf.u);
            printf("    y: %.9g  0x%08x\n", yf.f, yf.u);
            printf("    expected: %.9g  0x%08x\n", ef.f, ef.u);
            printf("    actual: %.9g  0x%08x\n", af.f, af.u);
            exit(1);
        }
    }
}

static void test_sub_loop(void) {
    #ifdef __onramp__
    // TODO onramp doesn't have time() yet
    FILE* f = fopen("/dev/urandom", "rb");
    if (f) {
        unsigned x;
        (void)fread(&x, 1, sizeof(x), f);
        (void)fclose(f);
        srand(x);
    }
    #else
    srand(time(NULL));
    #endif

    for (size_t i = 0;; ++i) {
        if (i != 0 && (i % 100000) == 0) printf("testing float subtraction: %zi...\n",i);

        floatunion_t xf, yf, ef, af;
        xf.u = random_float();
        yf.u = random_float();
        ef.f = xf.f - yf.f;
        //printf("Subtracting %.9g (0x%x) - %.9g (0x%x), expected %.9g (0x%x)\n", xf.f, xf.u, yf.f, yf.u, ef.f, ef.u);
        af.u = __float_sub(xf.u, yf.u);
        //printf("Actual result: %.9g (0x%x)\n", af.f, af.u);

        if (ef.u != af.u) {
            printf("__float_sub() FAILED:\n");
            printf("    x: %.9g  0x%08x\n", xf.f, xf.u);
            printf("    y: %.9g  0x%08x\n", yf.f, yf.u);
            printf("    expected: %.9g  0x%08x\n", ef.f, ef.u);
            printf("    actual: %.9g  0x%08x\n", af.f, af.u);
            exit(1);
        }
    }
}
#endif

            #endif

int main(void) {
            // TODO sigaction is not properly supported on onramp yet
            #ifndef __onramp__
            // TODO this initializer is crashing cci/2 at 5fcf1c7d but doesn't
            // crash in a standalone test case. Need to figure out why.
    struct sigaction act = {.sa_handler = test_sigfpe};
    if (0 != sigaction(SIGFPE, &act, NULL)) {
        perror("sigaction(SIGFPE) failed");
        exit(100);
    }

    test_add();
    test_sub();
    test_eq();
    test_lt();
    test_lte();

    #ifndef __onramp__
    //test_add_loop();
    //test_sub_loop();
    #endif

    (void)f2u;
    (void)test_add_loop;
    (void)test_sub_loop;
            #endif
}
