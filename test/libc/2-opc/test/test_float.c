// The MIT License (MIT)
// Copyright (c) 2025-2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#ifdef __onramp__
    #include <__onramp/__arithmetic.h>
    #define onramp_strtof strtof
#endif
#ifndef __onramp__
    #include <time.h>
    #define strtof onramp_strtof

    // this test case can be compiled standalone with a native C compiler:
    // gcc -O -Wall -Wextra -fsanitize=address -g test/test_float.c -o /tmp/a && /tmp/a
    #include "../../../../core/libc/2-opc/src/float.c"

    #undef strtof
    #define native_strtof strtof
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

static void test_signbit_impl(float x, int expected) {
    test_sigfpe_count = 0;

    floatunion_t xf;
    xf.f = x;
    int actual = __float_signbit(xf.u);
    //printf("signbit(%.9g) expected %i, got %i\n", x, expected, actual);

    if (test_sigfpe_count) {
        fprintf(stderr, "signbit(%.9g) signaled.\n", x);
        exit(1);
    }

    if (expected != actual) {
        fprintf(stderr, "signbit(%.9g) incorrect: expected %i, got %i\n",
                x, expected, actual);
        exit(1);
    }
}

static void test_signbit() {
    test_signbit_impl(0.0f, 0);
    test_signbit_impl(-0.0f, 1);
    test_signbit_impl(1.0f, 0);
    test_signbit_impl(-1.0f, 1);
    test_signbit_impl((float)INFINITY, 0);
    test_signbit_impl(-(float)INFINITY, 1);
    test_signbit_impl(u2f(0x00000001), 0); // subnormal
    test_signbit_impl(u2f(0x80000001), 1); // subnormal
    test_signbit_impl(u2f(0x7FFFFFFF), 0); // signaling nan (shouldn't signal)
    test_signbit_impl(u2f(0xFFFFFFFF), 1); // signaling nan (shouldn't signal)
}

static void test_copysignf_impl(float x, float y, float expected) {
    test_sigfpe_count = 0;

    floatunion_t xf, yf, ef, af;
    xf.f = x;
    yf.f = y;
    ef.f = expected;
    af.u = copysignf(xf.u, yf.u);
    //printf("copysignf(%.9g, %.9g) expected %.9g, got %.9g\n", x, y, expected, af.f);

    if (test_sigfpe_count) {
        fprintf(stderr, "copysignf(%.9g, %.9g) signaled.\n", x, y);
        exit(1);
    }

    if (ef.u != af.u) {
        fprintf(stderr, "copysignf(%.9g, %.9g) incorrect: expected %.9g, got %.9g\n",
                x, y, expected, af.f);
        exit(1);
    }
}

static void test_copysignf() {
    test_copysignf_impl(0.0f, 0.0f, 0.0f);
    test_copysignf_impl(-0.0f, 0.0f, 0.0f);
    test_copysignf_impl(0.0f, -0.0f, -0.0f);
    test_copysignf_impl(-0.0f, -0.0f, -0.0f);

    test_copysignf_impl(5.0f, 1.0f, 5.0f);
    test_copysignf_impl(-5.0f, 1.0f, 5.0f);
    test_copysignf_impl(5.0f, -1.0f, -5.0f);
    test_copysignf_impl(-5.0f, -1.0f, -5.0f);

    // test all combinations

    test_copysignf_impl(0.0f, 0.0f, 0.0f);
    test_copysignf_impl(0.0f, -0.0f, -0.0f);
    test_copysignf_impl(0.0f, u2f(0x00000001), 0.0f); // subnormal
    test_copysignf_impl(0.0f, u2f(0x80000001), -0.0f); // subnormal
    test_copysignf_impl(0.0f, (float)INFINITY, 0.0f);
    test_copysignf_impl(0.0f, -(float)INFINITY, -0.0f);
    test_copysignf_impl(0.0f, u2f(0x7fffffff), 0.0f); // signaling nan (shouldn't signal)
    test_copysignf_impl(0.0f, u2f(0xffffffff), -0.0f); // signaling nan (shouldn't signal)

    test_copysignf_impl(-0.0f, 0.0f, 0.0f);
    test_copysignf_impl(-0.0f, -0.0f, -0.0f);
    test_copysignf_impl(-0.0f, u2f(0x00000001), 0.0f); // subnormal
    test_copysignf_impl(-0.0f, u2f(0x80000001), -0.0f); // subnormal
    test_copysignf_impl(-0.0f, (float)INFINITY, 0.0f);
    test_copysignf_impl(-0.0f, -(float)INFINITY, -0.0f);
    test_copysignf_impl(-0.0f, u2f(0x7fffffff), 0.0f); // signaling nan (shouldn't signal)
    test_copysignf_impl(-0.0f, u2f(0xffffffff), -0.0f); // signaling nan (shouldn't signal)

    test_copysignf_impl(5.0f, 0.0f, 5.0f);
    test_copysignf_impl(5.0f, -0.0f, -5.0f);
    test_copysignf_impl(5.0f, u2f(0x00000001), 5.0f); // subnormal
    test_copysignf_impl(5.0f, u2f(0x80000001), -5.0f); // subnormal
    test_copysignf_impl(5.0f, (float)INFINITY, 5.0f);
    test_copysignf_impl(5.0f, -(float)INFINITY, -5.0f);
    test_copysignf_impl(5.0f, u2f(0x7fffffff), 5.0f); // signaling nan (shouldn't signal)
    test_copysignf_impl(5.0f, u2f(0xffffffff), -5.0f); // signaling nan (shouldn't signal)

    test_copysignf_impl(-5.0f, 0.0f, 5.0f);
    test_copysignf_impl(-5.0f, -0.0f, -5.0f);
    test_copysignf_impl(-5.0f, u2f(0x00000001), 5.0f); // subnormal
    test_copysignf_impl(-5.0f, u2f(0x80000001), -5.0f); // subnormal
    test_copysignf_impl(-5.0f, (float)INFINITY, 5.0f);
    test_copysignf_impl(-5.0f, -(float)INFINITY, -5.0f);
    test_copysignf_impl(-5.0f, u2f(0x7fffffff), 5.0f); // signaling nan (shouldn't signal)
    test_copysignf_impl(-5.0f, u2f(0xffffffff), -5.0f); // signaling nan (shouldn't signal)

    test_copysignf_impl(u2f(0x00000001), 0.0f, u2f(0x00000001));
    test_copysignf_impl(u2f(0x00000001), -0.0f, u2f(0x80000001));
    test_copysignf_impl(u2f(0x00000001), u2f(0x00000001), u2f(0x00000001));
    test_copysignf_impl(u2f(0x00000001), u2f(0x80000001), u2f(0x80000001));
    test_copysignf_impl(u2f(0x00000001), (float)INFINITY, u2f(0x00000001));
    test_copysignf_impl(u2f(0x00000001), -(float)INFINITY, u2f(0x80000001));
    test_copysignf_impl(u2f(0x00000001), u2f(0x7fffffff), u2f(0x00000001));
    test_copysignf_impl(u2f(0x00000001), u2f(0xffffffff), u2f(0x80000001));

    test_copysignf_impl(u2f(0x80000001), 0.0f, u2f(0x00000001));
    test_copysignf_impl(u2f(0x80000001), -0.0f, u2f(0x80000001));
    test_copysignf_impl(u2f(0x80000001), u2f(0x00000001), u2f(0x00000001));
    test_copysignf_impl(u2f(0x80000001), u2f(0x80000001), u2f(0x80000001));
    test_copysignf_impl(u2f(0x80000001), (float)INFINITY, u2f(0x00000001));
    test_copysignf_impl(u2f(0x80000001), -(float)INFINITY, u2f(0x80000001));
    test_copysignf_impl(u2f(0x80000001), u2f(0x7fffffff), u2f(0x00000001));
    test_copysignf_impl(u2f(0x80000001), u2f(0xffffffff), u2f(0x80000001));

    test_copysignf_impl((float)INFINITY, 0.0f, (float)INFINITY);
    test_copysignf_impl((float)INFINITY, -0.0f, -(float)INFINITY);
    test_copysignf_impl((float)INFINITY, u2f(0x00000001), (float)INFINITY); // subnormal
    test_copysignf_impl((float)INFINITY, u2f(0x80000001), -(float)INFINITY); // subnormal
    test_copysignf_impl((float)INFINITY, (float)INFINITY, (float)INFINITY);
    test_copysignf_impl((float)INFINITY, -(float)INFINITY, -(float)INFINITY);
    test_copysignf_impl((float)INFINITY, u2f(0x7fffffff), (float)INFINITY); // signaling nan (shouldn't signal)
    test_copysignf_impl((float)INFINITY, u2f(0xffffffff), -(float)INFINITY); // signaling nan (shouldn't signal)

    test_copysignf_impl(-(float)INFINITY, 0.0f, (float)INFINITY);
    test_copysignf_impl(-(float)INFINITY, -0.0f, -(float)INFINITY);
    test_copysignf_impl(-(float)INFINITY, u2f(0x00000001), (float)INFINITY); // subnormal
    test_copysignf_impl(-(float)INFINITY, u2f(0x80000001), -(float)INFINITY); // subnormal
    test_copysignf_impl(-(float)INFINITY, (float)INFINITY, (float)INFINITY);
    test_copysignf_impl(-(float)INFINITY, -(float)INFINITY, -(float)INFINITY);
    test_copysignf_impl(-(float)INFINITY, u2f(0x7fffffff), (float)INFINITY); // signaling nan (shouldn't signal)
    test_copysignf_impl(-(float)INFINITY, u2f(0xffffffff), -(float)INFINITY); // signaling nan (shouldn't signal)

    test_copysignf_impl(u2f(0x7FFFFFFF), 0.0f, u2f(0x7FFFFFFF));
    test_copysignf_impl(u2f(0x7FFFFFFF), -0.0f, u2f(0xFFFFFFFF));
    test_copysignf_impl(u2f(0x7FFFFFFF), u2f(0x00000001), u2f(0x7FFFFFFF)); // subnormal
    test_copysignf_impl(u2f(0x7FFFFFFF), u2f(0x80000001), u2f(0xFFFFFFFF)); // subnormal
    test_copysignf_impl(u2f(0x7FFFFFFF), (float)INFINITY, u2f(0x7FFFFFFF));
    test_copysignf_impl(u2f(0x7FFFFFFF), -(float)INFINITY, u2f(0xFFFFFFFF));
    test_copysignf_impl(u2f(0x7FFFFFFF), u2f(0x7fffffff), u2f(0x7FFFFFFF)); // signaling nan (shouldn't signal)
    test_copysignf_impl(u2f(0x7FFFFFFF), u2f(0xffffffff), u2f(0xFFFFFFFF)); // signaling nan (shouldn't signal)

    test_copysignf_impl(u2f(0xFFFFFFFF), 0.0f, u2f(0x7FFFFFFF));
    test_copysignf_impl(u2f(0xFFFFFFFF), -0.0f, u2f(0xFFFFFFFF));
    test_copysignf_impl(u2f(0xFFFFFFFF), u2f(0x00000001), u2f(0x7FFFFFFF)); // subnormal
    test_copysignf_impl(u2f(0xFFFFFFFF), u2f(0x80000001), u2f(0xFFFFFFFF)); // subnormal
    test_copysignf_impl(u2f(0xFFFFFFFF), (float)INFINITY, u2f(0x7FFFFFFF));
    test_copysignf_impl(u2f(0xFFFFFFFF), -(float)INFINITY, u2f(0xFFFFFFFF));
    test_copysignf_impl(u2f(0xFFFFFFFF), u2f(0x7fffffff), u2f(0x7FFFFFFF)); // signaling nan (shouldn't signal)
    test_copysignf_impl(u2f(0xFFFFFFFF), u2f(0xffffffff), u2f(0xFFFFFFFF)); // signaling nan (shouldn't signal)
}

static void test_fpclassify_impl(float x, int expected) {
    test_sigfpe_count = 0;

    floatunion_t xf;
    xf.f = x;
    int actual = __float_fpclassify(xf.u);
    //printf("fpclassify(%.9g) expected %i, got %i\n", x, expected, actual);

    if (test_sigfpe_count) {
        fprintf(stderr, "fpclassify(%.9g) signaled.\n", x);
        exit(1);
    }

    if (expected != actual) {
        fprintf(stderr, "fpclassify(%.9g) incorrect: expected %i, got %i\n",
                x, expected, actual);
        exit(1);
    }
}

static void test_fpclassify() {
    test_fpclassify_impl(0.0f, FP_ZERO);
    test_fpclassify_impl(-0.0f, FP_ZERO);
    test_fpclassify_impl(u2f(0x00000001), FP_SUBNORMAL);
    test_fpclassify_impl(u2f(0x80000001), FP_SUBNORMAL);
    test_fpclassify_impl(5.0f, FP_NORMAL);
    test_fpclassify_impl(-5.0f, FP_NORMAL);
    test_fpclassify_impl(u2f(0x3FFFFFFF), FP_NORMAL);
    test_fpclassify_impl(u2f(0xBFFFFFFF), FP_NORMAL);
    test_fpclassify_impl((float)INFINITY, FP_INFINITE);
    test_fpclassify_impl(-(float)INFINITY, FP_INFINITE);
    test_fpclassify_impl(u2f(0x7FFFFFFF), FP_NAN);
    test_fpclassify_impl(u2f(0xFFFFFFFF), FP_NAN);
}

static void test_issignaling_impl(float x, int expected) {
    test_sigfpe_count = 0;

    floatunion_t xf;
    xf.f = x;
    int actual = __float_issignaling(xf.u);
    //printf("issignaling(%.9g) expected %i, got %i\n", x, expected, actual);

    if (test_sigfpe_count) {
        fprintf(stderr, "issignaling(%.9g) signaled.\n", x);
        exit(1);
    }

    if (expected != actual) {
        fprintf(stderr, "issignaling(%.9g) incorrect: expected %i, got %i\n",
                x, expected, actual);
        exit(1);
    }
}

static void test_issignaling() {
    test_issignaling_impl(0.0f, 0);
    test_issignaling_impl(-0.0f, 0);
    test_issignaling_impl(u2f(0x00000001), 0);
    test_issignaling_impl(u2f(0x80000001), 0);
    test_issignaling_impl(5.0f, 0);
    test_issignaling_impl(-5.0f, 0);
    test_issignaling_impl(u2f(0x3FFFFFFF), 0);
    test_issignaling_impl(u2f(0xBFFFFFFF), 0);
    test_issignaling_impl((float)INFINITY, 0);
    test_issignaling_impl(-(float)INFINITY, 0);
    test_issignaling_impl(u2f(0x7FBFFFFF), 1); // top significand bit is the quiet bit
    test_issignaling_impl(u2f(0xFFBFFFFF), 1);
    test_issignaling_impl(u2f(0x7FFFFFFF), 0);
    test_issignaling_impl(u2f(0xFFFFFFFF), 0);
}

/* TODO these are all just trivial macros that wrap fpclassify(). We don't
 * currently have a way to test them because when compiling natively we get
 * native headers with native macros. These will need to be tested only on
 * Onramp once we can build this with cci/2 and libc/2. */
#ifdef __onramp__
static void test_isinf() {
}

static void test_isnan() {
}

static void test_isnormal() {
}

static void test_isfinite() {
}

static void test_iszero() {
}

static void test_issubnormal() {
}
#endif

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

static void test_strtof_case(const char* str, float expected_float, uint32_t expected_bits, int error) {
    if (f2u(expected_float) != expected_bits) {
        printf("invalid test, expected float %.9g has bits %#x, expected bits %#x is float %.9g\n",
                expected_float, f2u(expected_float), expected_bits, u2f(expected_bits));
        exit(1);
    }

    char* end;

    #ifndef __onramp__
    // compare to native libc strtof
    errno = 0;
    float nf = native_strtof(str, &end);
    if (end != str + strlen(str)) {
        printf("native failed to parse %s\n", str);
        exit(1);
    }
    if (f2u(nf) != expected_bits) {
        printf("native strtof(\"%s\") == %.9g %#x, expected %.9g %#x\n",
                str, nf, f2u(nf), expected_float, expected_bits);
        exit(1);
    }
    if (errno != error) {
        printf("native %s raised incorrect error code: expected %i, actual %i\n", str, error, errno);
        exit(1);
    }
    #endif

    // compare to onramp strtof
    errno = 0;
    uint32_t of = onramp_strtof(str, &end);
    if (end != str + strlen(str)) {
        printf("onramp failed to parse %s\n", str);
        exit(1);
    }
    if (of != expected_bits) {
        printf("onramp strtof(\"%s\") == %.9g %#x, expected %.9g %#x\n",
                str, u2f(of), of, expected_float, expected_bits);
        exit(1);
    }
    if (errno != error) {
        printf("onramp %s raised incorrect error code: expected %i, actual %i\n", str, error, errno);
        exit(1);
    }
}

static void test_strtof(void) {

    // small integers
    test_strtof_case("1", 1.f, 0b0'01111111'00000000000000000000000, 0);
    test_strtof_case("2.0", 2.0f, 0b0'10000000'00000000000000000000000, 0);
    test_strtof_case("3e0", 3e0f, 0b0'10000000'10000000000000000000000, 0);
    test_strtof_case("4.0e0", 4.0e0f, 0b0'10000001'00000000000000000000000, 0);
    test_strtof_case("1e1", 1e1f, 0b0'10000010'01000000000000000000000, 0); // 10
    test_strtof_case("1000.00e-1", 1000.00e-1f, 0b0'10000101'10010000000000000000000, 0); // 100

    // integer boundary cases
    test_strtof_case("8388607",   8388607.0f, 0b0'10010101'11111111111111111111110, 0); // exact
    test_strtof_case("8388607.5", 8388607.5f, 0b0'10010101'11111111111111111111111, 0); // exact
    test_strtof_case("8388608",   8388608.0f, 0b0'10010110'00000000000000000000000, 0); // exact
    test_strtof_case("8388608.5", 8388608.5f, 0b0'10010110'00000000000000000000000, 0); // rounds to 8388608
    test_strtof_case("8388609",   8388609.0f, 0b0'10010110'00000000000000000000001, 0); // exact
    test_strtof_case("16777215", 16777215.0f, 0b0'10010110'11111111111111111111111, 0); // exact
    test_strtof_case("16777216", 16777216.0f, 0b0'10010111'00000000000000000000000, 0); // exact
    test_strtof_case("16777217", 16777217.0f, 0b0'10010111'00000000000000000000000, 0); // rounds to 16777216

    // integers too large
    test_strtof_case("602214076000000000000000", 602214076000000000000000.f, 0b0'11001101'11111110000110000101110, 0); // 1 mol

    // small fractions
    test_strtof_case("0.5", 0.5f, 0b0'01111110'00000000000000000000000, 0);
    test_strtof_case("0.25", 0.25f, 0b0'01111101'00000000000000000000000, 0);
    test_strtof_case("0.125", 0.125f, 0b0'01111100'00000000000000000000000, 0);
    test_strtof_case("0.1", 0.1f, 0b0'01111011'10011001100110011001101, 0);
    test_strtof_case("10.00000000000000000e-2", 10.00000000000000000e-2f, 0b0'01111011'10011001100110011001101, 0);
    test_strtof_case("0.000001", 0.000001f, 0b0'01101011'00001100011011110111101, 0);
    test_strtof_case("0.000000000000000000000000000001", 0.000000000000000000000000000001f, 0b0'00011011'01000100100001001100000, 0);
    test_strtof_case("0.0000000000000000000000000000000000001", 0.0000000000000000000000000000000000001f, 0b0'00000100'00010000001110011101010, 0);
    test_strtof_case("1.602176634e-19", 1.602176634e-19f, 0b0'01000000'01111010010011011010001, 0); // 1 eV in J

    // misc decimal numbers
    test_strtof_case("123.456", 123.456f, 0b0'10000101'11101101110100101111001, 0);
    test_strtof_case("3.14159265358", 3.14159265358f, 0b0'10000000'10010010000111111011011, 0);

    // subnormals
    test_strtof_case("1.175494351e-38", 1.175494351e-38, 0b0'00000001'00000000000000000000000, 0); // smallest normalized number (FLT_MIN)
    test_strtof_case("1.175494211e-38", 1.175494211e-38, 0b0'00000000'11111111111111111111111, ERANGE); // largest subnormal number
    test_strtof_case("9.999946101e-41", 9.999946101e-41, 0b0'00000000'00000010001011011000010, ERANGE);
    test_strtof_case("1e-40", 1e-40f, 0b0'00000000'00000010001011011000010, ERANGE);
    test_strtof_case("1e-42", 1e-42f, 0b0'00000000'00000000000001011001010, ERANGE);
    test_strtof_case("1.401298464e-45", 1.401298464e-45, 0b0'00000000'00000000000000000000001, ERANGE); // smallest subnormal
    test_strtof_case("1e-45", 1e-45f, 0b0'00000000'00000000000000000000001, ERANGE); // also smallest subnormal
    test_strtof_case("1e-46", 0 /*avoid gcc warning on 1e-46f*/, 0, ERANGE); // too small to be representable

    // zero
    test_strtof_case("0", 0, 0, 0);
    test_strtof_case("00000", 00000, 0, 0);
    test_strtof_case("0.0", 0.0f, 0, 0);
    test_strtof_case("00000.0000000000000", 00000.0000000000000f, 0, 0);

    // negative numbers, whitespace
    test_strtof_case("-0.1", -0.1f, 0b1'011110111'0011001100110011001101, 0);
    test_strtof_case("    -123.456", -123.456f, 0b1'10000101'11101101110100101111001, 0);
    test_strtof_case("\t\t\t-3.14159265358", -3.14159265358f, 0b1'10000000'10010010000111111011011, 0);
    test_strtof_case("-0", -0.f, 0b1'000000000'0000000000000000000000, 0);

    // explicit plus sign, whitespace
    test_strtof_case("+0.1", +0.1f, 0b0'01111011'10011001100110011001101, 0);
    test_strtof_case(" \t \t+123.456", +123.456f, 0b0'10000101'11101101110100101111001, 0);
    test_strtof_case("\t    \t   +3.14159265358", +3.14159265358f, 0b0'10000000'10010010000111111011011, 0);
    test_strtof_case("+0", +0.f, 0, 0);

    // infinity
    test_strtof_case("inf", INFINITY, 0b0'11111111'00000000000000000000000, 0);
    test_strtof_case("infinity", INFINITY, 0b0'11111111'00000000000000000000000, 0);
    test_strtof_case("INF", INFINITY, 0b0'11111111'00000000000000000000000, 0);
    test_strtof_case("INFINITY", INFINITY, 0b0'11111111'00000000000000000000000, 0);
    test_strtof_case("InF", INFINITY, 0b0'11111111'00000000000000000000000, 0);
    test_strtof_case("iNfInItY", INFINITY, 0b0'11111111'00000000000000000000000, 0);
    test_strtof_case("+inf", +INFINITY, 0b0'11111111'00000000000000000000000, 0);
    test_strtof_case("-INFINITY", -INFINITY, 0b1'11111111'00000000000000000000000, 0);

    // nan
    test_strtof_case("nan", NAN, 0b0'11111111'10000000000000000000000, 0);
    test_strtof_case("NAN", NAN, 0b0'11111111'10000000000000000000000, 0);
    test_strtof_case("NaN", NAN, 0b0'11111111'10000000000000000000000, 0);
    test_strtof_case("nAn", NAN, 0b0'11111111'10000000000000000000000, 0);

    // TODO test that it rejects errors correctly

}

static void test_float_from_u32_impl(unsigned value, float expected_float, unsigned expected_bits) {
    if (f2u(expected_float) != expected_bits) {
        printf("invalid test: expected float %.9g with bits %#x does not match expected bits %#x\n",
                expected_float, f2u(expected_float), expected_bits);
        exit(1);
    }

    #ifndef __onramp__
    if (expected_bits != f2u((float)value)) {
        printf("u32 %u has float value %.9g %#x, does not match expected %.9g %#x",
                value, (float)value, f2u((float)value), expected_float, expected_bits);
        exit(1);
    }
    #endif

    uint32_t result = __float_from_u32(value);
    if (expected_bits != result) {
        printf("__float_from_u32(%u) returned %.9g %#x, expected %.9g %#x\n",
                value, u2f(result), result, expected_float, expected_bits);
        exit(1);
    }
}

static void test_float_from_u32(void) {
    test_float_from_u32_impl(0u, 0.0f, 0);
    test_float_from_u32_impl(1u, 1.0f, 0b0'01111111'00000000000000000000000);
    test_float_from_u32_impl(2u, 2.0f, 0b0'10000000'00000000000000000000000);
    test_float_from_u32_impl(3u, 3.0f, 0b0'10000000'10000000000000000000000);
    test_float_from_u32_impl(4u, 4.0f, 0b0'10000001'00000000000000000000000);
    test_float_from_u32_impl(123456u, 123456.0f, 0b0'10001111'11100010010000000000000);
    test_float_from_u32_impl(8388609u, 8388609.0f, 0b0'10010110'00000000000000000000001); // exact
    test_float_from_u32_impl(16777215u, 16777215.0f, 0b0'10010110'11111111111111111111111); // exact
    test_float_from_u32_impl(16777216u, 16777216.0f, 0b0'10010111'00000000000000000000000); // exact
    test_float_from_u32_impl(16777217u, 16777217.0f, 0b0'10010111'00000000000000000000000); // rounded to even
    test_float_from_u32_impl(16777218u, 16777218.0f, 0b0'10010111'00000000000000000000001); // exact
    test_float_from_u32_impl(33554432u, 33554432.0f, 0b0'10011000'00000000000000000000000); // exact
    test_float_from_u32_impl(44444444u, 44444444.0f, 0b0'10011000'01010011000101011000111); // apparently exact
    test_float_from_u32_impl(67108864u, 67108864.0f, 0b0'10011001'00000000000000000000000); // exact
    test_float_from_u32_impl(4294967295, 4294967295.0f, 0b0'10011111'00000000000000000000000); // rounded, UINT32_MAX

    // try all numbers (recommend adding -O3 for this)
    #ifdef DISABLED
    for (uint32_t i = 1; i != 0; ++i) {
        if ((i % 50000000) == 0) printf("testing __float_from_u32(): %u... (%f%%)\n",i, 100.0*(double)i/(double)UINT32_MAX);
        test_float_from_u32_impl(i, (float)i, f2u((float)i));
    }
    printf("testing __float_from_u32(): done.\n");
    #endif
}

static void test_float_from_i32_impl(int value, float expected_float, unsigned expected_bits) {
    if (f2u(expected_float) != expected_bits) {
        printf("invalid test: expected float %.9g with bits %#x does not match expected bits %#x\n",
                expected_float, f2u(expected_float), expected_bits);
        exit(1);
    }

    #ifndef __onramp__
    if (expected_bits != f2u((float)value)) {
        printf("u32 %u has float value %.9g %#x, does not match expected %.9g %#x",
                value, (float)value, f2u((float)value), expected_float, expected_bits);
        exit(1);
    }
    #endif

    uint32_t result = __float_from_i32(value);
    if (expected_bits != result) {
        printf("__float_from_u32(%i) returned %.9g %#x, expected %.9g %#x\n",
                value, u2f(result), result, expected_float, expected_bits);
        exit(1);
    }
}

static void test_float_from_i32(void) {
    // some tests from unsigned
    test_float_from_i32_impl(1, 1.0f, 0b0'01111111'00000000000000000000000);
    test_float_from_i32_impl(123456, 123456.0f, 0b0'10001111'11100010010000000000000);
    test_float_from_i32_impl(33554432, 33554432.0f, 0b0'10011000'00000000000000000000000);

    // negative numbers
    test_float_from_i32_impl(-1, -1.0f, 0b1'01111111'00000000000000000000000);
    test_float_from_i32_impl(-123456, -123456.0f, 0b1'10001111'11100010010000000000000);
    test_float_from_i32_impl(-33554432, -33554432.0f, 0b1'10011000'00000000000000000000000);

    // TODO better tests, boundary conditions
}

static void test_float_from_u64_impl(uint64_t value, float expected_float, unsigned expected_bits) {
    if (f2u(expected_float) != expected_bits) {
        printf("invalid test: expected float %.9g with bits %#x does not match expected bits %#x\n",
                expected_float, f2u(expected_float), expected_bits);
        exit(1);
    }

    #ifndef __onramp__
    if (expected_bits != f2u((float)value)) {
        printf("u64 %" PRIu64 " has float value %.9g %#x, does not match expected %.9g %#x",
                value, (float)value, f2u((float)value), expected_float, expected_bits);
        exit(1);
    }
    #endif

    unsigned result = __float_from_u64((unsigned*)&value); // assumes little-endian, also illegal type-punned pointer
    if (expected_bits != result) {
        printf("__float_from_u64(%" PRIu64 ") returned %.9g %#x, expected %.9g %#x\n",
                value, u2f(result), result, expected_float, expected_bits);
        exit(1);
    }
}

static void test_float_from_u64(void) {

    // same tests as u32
    test_float_from_u64_impl(0u, 0.0f, 0);
    test_float_from_u64_impl(1u, 1.0f, 0b0'01111111'00000000000000000000000);
    test_float_from_u64_impl(2u, 2.0f, 0b0'10000000'00000000000000000000000);
    test_float_from_u64_impl(3u, 3.0f, 0b0'10000000'10000000000000000000000);
    test_float_from_u64_impl(4u, 4.0f, 0b0'10000001'00000000000000000000000);
    test_float_from_u64_impl(123456u, 123456.0f, 0b0'10001111'11100010010000000000000);
    test_float_from_u64_impl(8388609u, 8388609.0f, 0b0'10010110'00000000000000000000001); // exact
    test_float_from_u64_impl(16777215u, 16777215.0f, 0b0'10010110'11111111111111111111111); // exact
    test_float_from_u64_impl(16777216u, 16777216.0f, 0b0'10010111'00000000000000000000000); // exact
    test_float_from_u64_impl(16777217u, 16777217.0f, 0b0'10010111'00000000000000000000000); // rounded to even
    test_float_from_u64_impl(16777218u, 16777218.0f, 0b0'10010111'00000000000000000000001); // exact
    test_float_from_u64_impl(33554432u, 33554432.0f, 0b0'10011000'00000000000000000000000); // exact
    test_float_from_u64_impl(44444444u, 44444444.0f, 0b0'10011000'01010011000101011000111); // apparently exact
    test_float_from_u64_impl(67108864u, 67108864.0f, 0b0'10011001'00000000000000000000000); // exact
    test_float_from_u64_impl(4294967295, 4294967295.0f, 0b0'10011111'00000000000000000000000); // rounded, UINT64_MAX

    // tests of u64
    test_float_from_u64_impl(4294967296ULL, 4294967296.0f, 0b0'10011111'00000000000000000000000); // rounded, UINT64_MAX+1
    test_float_from_u64_impl(1234567890123ULL, 1.23456795e+12, 0b0'10100111'00011111011100011111110);
    test_float_from_u64_impl(1234567890123456ULL, 1.23456795e+15, 0b0'10110001'00011000101101010101000);
    test_float_from_u64_impl(8765432187654321ULL, 8.76543232e+15, 0b0'10110011'11110010010000011101010); // less than 58 bits
    test_float_from_u64_impl(144115188075855872ULL, 144115188075855872.0f, 0b0'10111000'00000000000000000000000); // exactly 58 bits, 26-bit high word, smallest value
    test_float_from_u64_impl(288230376151711743ULL, 288230376151711743.0f, 0b0'10111001'00000000000000000000000); // exactly 58 bits, 26-bit high word, largest value
    test_float_from_u64_impl(987654321987654321ULL, 987654321987654321.0f, 0b0'10111010'10110110100110110100110); // more than 58 bits

    // bugs uncovered from random testing
    test_float_from_u64_impl(75433444109690124ULL, 75433444109690124.0f, 0b0'10110111'00001011111111100101001);

    // random testing
    #ifdef DISABLED
    for (size_t i = 0;; ++i) {
        if ((i % 50000000) == 0) printf("testing __float_from_u64(): %zu...\n", i);
        uint64_t value;
        arc4random_buf(&value, sizeof(value)); // random value
        value >>= (arc4random() & 63); // random number of bits
        test_float_from_u64_impl(value, (float)value, f2u((float)value));
    }
    printf("testing __float_from_u64(): done.\n");
    #endif
}

static void test_float_from_i64_impl(int64_t value, float expected_float, unsigned expected_bits) {
    if (f2u(expected_float) != expected_bits) {
        printf("invalid test: expected float %.9g with bits %#x does not match expected bits %#x\n",
                expected_float, f2u(expected_float), expected_bits);
        exit(1);
    }

    #ifndef __onramp__
    if (expected_bits != f2u((float)value)) {
        printf("u64 %" PRId64 " has float value %.9g %#x, does not match expected %.9g %#x",
                value, (float)value, f2u((float)value), expected_float, expected_bits);
        exit(1);
    }
    #endif

    unsigned result = __float_from_i64((unsigned*)&value); // assumes little-endian, also illegal type-punned pointer
    if (expected_bits != result) {
        printf("__float_from_u64(%" PRId64 ") returned %.9g %#x, expected %.9g %#x\n",
                value, u2f(result), result, expected_float, expected_bits);
        exit(1);
    }
}

static void test_float_from_i64(void) {
    // some tests from unsigned
    test_float_from_i64_impl(1, 1.0f, 0b0'01111111'00000000000000000000000);
    test_float_from_i64_impl(123456, 123456.0f, 0b0'10001111'11100010010000000000000);
    test_float_from_i64_impl(33554432, 33554432.0f, 0b0'10011000'00000000000000000000000);
    test_float_from_i64_impl(288230376151711743LL, 288230376151711743.0f, 0b0'10111001'00000000000000000000000);

    // negative numbers
    test_float_from_i64_impl(-1, -1.0f, 0b1'01111111'00000000000000000000000);
    test_float_from_i64_impl(-123456, -123456.0f, 0b1'10001111'11100010010000000000000);
    test_float_from_i64_impl(-33554432, -33554432.0f, 0b1'10011000'00000000000000000000000);
    test_float_from_i64_impl(-288230376151711743LL, -288230376151711743.0f, 0b1'10111001'00000000000000000000000);

    // TODO better tests, boundary conditions
}

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

    // arithmetic
    test_add();
    test_sub();

    // comparisons
    test_eq();
    test_lt();
    test_lte();

    // classification
    test_signbit();
    test_copysignf();
    test_fpclassify();
    test_issignaling();
    #ifdef __onramp__
    test_isinf();
    test_isnan();
    test_isnormal();
    test_isfinite();
    test_iszero();
    test_issubnormal();
    #endif

    // conversion
    test_float_from_u32();
    test_float_from_i32();
    test_float_from_u64();
    test_float_from_i64();

    // parse
    test_strtof();

    (void)f2u;

    // long-running tests
    #ifndef __onramp__
    //test_add_loop();
    //test_sub_loop();
    #endif
    (void)test_add_loop;
    (void)test_sub_loop;
            #endif
}
