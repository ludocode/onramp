// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __onramp__
    #include <__onramp/__arithmetic.h>
#endif
#ifndef __onramp__
    // this test case can be compiled standalone with a native C compiler or onrampcc:
    // gcc -DTEST_DIV_LOOP -Wall -Wextra -Wpedantic -fsanitize=address -g test/test_llong.c -o /tmp/a && /tmp/a
    #include "../../../../core/libc/2-opc/src/llong.c"
    #include <time.h>
#endif

void __llong_divmodu(unsigned* quotient_out, unsigned* /*nullable*/ remainder_out,
        const unsigned* dividend, const unsigned* divisor);

static void test_add(void) {
    unsigned long long a = 0x0123456789abcdefULL;
    unsigned long long b = 0x9abcdef012345678ULL;
    __llong_add((unsigned*)&a, (unsigned*)&a, (unsigned*)&b);
    if (a != 0x9be024579be02467ULL) {
        exit(1);
    }
}

static void test_sub(void) {
    unsigned long long a = 0x0123456789abcdefULL;
    unsigned long long b = 0x34567890abcdef12ULL;
    __llong_sub((unsigned*)&a, (unsigned*)&a, (unsigned*)&b);
    if (a != 0xccccccd6dddddeddULL) {
        exit(1);
    }
}

static void test_mul(void) {
    unsigned long long a = 0x9abcdef012345678ULL;
    unsigned long long b = 0x34567890abcdef12ULL;
    __llong_mul((unsigned*)&a, (unsigned*)&a, (unsigned*)&b);
    if (a != 0x4a1a3b0b8a801c70ULL) {
        exit(1);
    }
}

static void test_divs(void) {
    long long a = -0x0c760000LL;
    long long b = 0x0a00000LL;
    __llong_divs((unsigned*)&a, (unsigned*)&a, (unsigned*)&b);
    if (a != -0x13) {
        exit(1);
    }

    // TODO, other numbers were tested with test_div_loop() below
}

static void test_divu(void) {
    unsigned long long a = 0x0c760000ULL;
    unsigned long long b = 0x0a00000ULL;
    __llong_divu((unsigned*)&a, (unsigned*)&a, (unsigned*)&b);
    if (a != 0x13ULL) {
        exit(1);
    }

    // TODO, other numbers were tested with test_div_loop() below
}

static void test_mods(void) {
    // TODO
}

static void test_modu(void) {
    // TODO
}

static void test_ltu(void) {
    unsigned long long a = 0x9abcdef012345678ULL;
    unsigned long long b = 0x34567890abcdef12ULL;
    if (__llong_ltu((unsigned*)&a, (unsigned*)&b)) {
        exit(1);
    }
    if (!__llong_ltu((unsigned*)&b, (unsigned*)&a)) {
        exit(1);
    }
    if (__llong_ltu((unsigned*)&a, (unsigned*)&a)) {
        exit(1);
    }
}

static void test_shl(void) {
    unsigned long long a;
    unsigned long long b = 0x9abcdef012345678ULL;
    __llong_shl((unsigned*)&a, (unsigned*)&b, 0);
    if (a != b) {
        exit(1);
    }
    __llong_shl((unsigned*)&a, (unsigned*)&b, 4);
    if (a != 0xabcdef0123456780ULL) {
        exit(1);
    }
    __llong_shl((unsigned*)&a, (unsigned*)&b, 32);
    if (a != 0x1234567800000000ULL) {
        exit(1);
    }
    __llong_shl((unsigned*)&a, (unsigned*)&b, 52);
    if (a != 0x6780000000000000ULL) {
        exit(1);
    }

    a = 1;
    __llong_shl((unsigned*)&a, (unsigned*)&a, 63);
    if (a != 0x8000000000000000ULL) {
        exit(1);
    }
}

static void test_shru(void) {
    unsigned long long a;
    unsigned long long b = 0x9abcdef012345678ULL;
    __llong_shru((unsigned*)&a, (unsigned*)&b, 0);
    if (a != b) {
        exit(1);
    }
    __llong_shru((unsigned*)&a, (unsigned*)&b, 4);
    if (a != 0x9abcdef01234567ULL) {
        exit(1);
    }
    __llong_shru((unsigned*)&a, (unsigned*)&b, 32);
    if (a != 0x9abcdef0ULL) {
        exit(1);
    }
    __llong_shru((unsigned*)&a, (unsigned*)&b, 52);
    if (a != 0x9abULL) {
        exit(1);
    }
}

static void test_shrs(void) {

    // positive
    {
        unsigned long long a;
        unsigned long long b = 0x34567890abcdef12ULL;
        __llong_shrs((unsigned*)&a, (unsigned*)&b, 0);
        if (a != b) {
            exit(1);
        }
        __llong_shrs((unsigned*)&a, (unsigned*)&b, 4);
        if (a != 0x34567890abcdef1ULL) {
            exit(2);
        }
        __llong_shrs((unsigned*)&a, (unsigned*)&b, 32);
        if (a != 0x34567890ULL) {
            exit(3);
        }
        __llong_shrs((unsigned*)&a, (unsigned*)&b, 52);
        if (a != 0x345ULL) {
            exit(4);
        }
    }

    // negative
    {
        unsigned long long a;
        unsigned long long b = 0x9abcdef012345678ULL;
        __llong_shrs((unsigned*)&a, (unsigned*)&b, 0);
        if (a != b) {
            exit(5);
        }
        __llong_shrs((unsigned*)&a, (unsigned*)&b, 4);
        if (a != 0xf9abcdef01234567ULL) {
            exit(6);
        }
        __llong_shrs((unsigned*)&a, (unsigned*)&b, 32);
        if (a != 0xffffffff9abcdef0ULL) {
            exit(7);
        }
        __llong_shrs((unsigned*)&a, (unsigned*)&b, 52);
        if (a != 0xfffffffffffff9abULL) {
            exit(8);
        }
    }
}

static void test_bit_and(void) {
    unsigned long long a = 0x34567890abcdef12ULL;
    unsigned long long b = 0x9abcdef012345678ULL;
    __llong_bit_and((unsigned*)&a, (unsigned*)&a, (unsigned*)&b);
    if (a != 0x1014589002044610ULL) {
        exit(1);
    }
}

static void test_bit_or(void) {
    unsigned long long a = 0x34567890abcdef12ULL;
    unsigned long long b = 0x9abcdef012345678ULL;
    __llong_bit_or((unsigned*)&a, (unsigned*)&a, (unsigned*)&b);
    if (a != 0xbefefef0bbfdff7aULL) {
        exit(1);
    }
}

static void test_bit_xor(void) {
    unsigned long long a = 0x34567890abcdef12ULL;
    unsigned long long b = 0x9abcdef012345678ULL;
    __llong_bit_xor((unsigned*)&a, (unsigned*)&a, (unsigned*)&b);
    if (a != 0xaeeaa660b9f9b96aULL) {
        exit(1);
    }
}

static void test_bit_not(void) {
    unsigned long long a = 0x34567890abcdef12ULL;
    __llong_bit_not((unsigned*)&a, (unsigned*)&a);
    if (a != 0xcba9876f543210edULL) {
        exit(1);
    }
}

static void test_negate(void) {
    unsigned long long a = 13;
    __llong_negate((unsigned*)&a, (unsigned*)&a);
    if (a != -13) exit(1);
}

#ifdef TEST_DIV_LOOP
static unsigned long long random_number(unsigned digits) {
    unsigned long long ret = 0;
    for (unsigned i = 0; i < digits; ++i) {
        ret <<= 16;
        ret |= (((unsigned)rand() & 0xff) << 8) | ((unsigned)rand() & 0xff);
    }
    return ret;
}

static void test_div_loop(void) {
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

    // test short
    for (size_t i = 0;; ++i) {
if (i != 0 && (i % 100000) == 0) printf("testing long long division: %zi...\n",i);
        unsigned long long dividend = random_number(1+rand()%4);
        unsigned long long divisor = random_number(1+rand()%4);
//divisor|=0x8000000000000000ull;
        if (divisor == 0)
            continue;
//dividend = 15514892400080185975u;
//divisor =  981662014;

        /*
        printf("__llong_divmodu() testing:\n");
        printf("    dividend: %llu\n", dividend);
        printf("    divisor: %llu\n", divisor);
        fflush(stdout);
        */

        unsigned long long quotient, remainder;
        __llong_divmodu(
                (unsigned*)&quotient,
                (unsigned*)&remainder,
                (unsigned*)&dividend,
                (unsigned*)&divisor);

        #ifdef __onramp__
        long long expected_remainder = (long long)(dividend - quotient * divisor);
        if (expected_remainder < 0 || expected_remainder >= divisor)
        #else
        if (quotient != dividend / divisor || remainder != dividend % divisor)
        #endif
        {
            printf("__llong_divmodu() FAILED:\n");
            printf("    dividend: %llu\n", dividend);
            printf("    divisor: %llu\n", divisor);
            #ifndef __onramp__
            printf("    expected quotient: %llu\n", dividend / divisor);
            printf("    expected remainder: %llu\n", dividend % divisor);
            #endif
            printf("    incorrect quotient: %llu\n", quotient);
            printf("    incorrect remainder: %llu\n", remainder);
            exit(1);
        }
//break;
    }

}
#endif

int main(void) {
    test_add();
    test_sub();
    test_mul();
    test_divs();
    test_divu();
    test_mods();
    test_modu();
    test_ltu();
    test_shl();
    test_shru();
    test_shrs();
    test_bit_and();
    test_bit_or();
    test_bit_xor();
    test_bit_not();
    test_negate();

    #ifdef TEST_DIV_LOOP
    test_div_loop();
    #endif
}
