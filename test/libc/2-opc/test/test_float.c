// The MIT License (MIT)
// Copyright (c) 2025 Fraser Heavy Software
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
    ++test_sigfpe_count;
}

static void test_add_impl(float x, float y, float expected) {
    floatunion_t xf, yf, ef, af;
    xf.f = x;
    yf.f = y;
    ef.f = expected;
    af.u = __float_add(xf.u, yf.u);
    if (ef.u != af.u) {
        fprintf(stderr, "Failed to add %.9g + %.9g: expected %.9g, got %.9g\n",
                x, y, ef.f, af.f);
        exit(1);
    }
}

static void test_add(void) {

    // integers less than 1<<24 should be exact
    test_add_impl(1.f, 4.f, 5.f);
    test_add_impl(47.f, 325.f, 372.f);

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
        //printf("Adding %.9g + %.9g, expected %.9g", xf.f, yf.f, ef.f);
        af.u = __float_add(xf.u, yf.u);
        //printf(", actual %.9g\n", af.f);

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
#endif

            #endif

int main(void) {
            // TODO sigaction is not properly supported on onramp yet
            #ifndef __onramp__
    struct sigaction act = {.sa_handler = test_sigfpe};
    if (0 != sigaction(SIGFPE, &act, NULL)) {
        perror("sigaction(SIGFPE) failed");
        exit(100);
    }

    test_add();

    #ifndef __onramp__
    test_add_loop();
    #endif
            #endif
}
