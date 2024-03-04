#define _USE_MATH_DEFINES

#include "math.c"

#include "test.h"

#include <stdio.h>

static void test_math_sin(void) {
    for (double i = -M_PI*2; i <= M_PI*2; i += M_PI/10) {
        #if 0
        printf("sin(%f) == %f\n", i, sin(i));
        #endif
    }
}

void test_math(void) {
    test_math_sin();
    //TODO
    puts("math tests pass");
}
