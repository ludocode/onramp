// The MIT License (MIT)
// Copyright (c) 2023-2024 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// TODO tests currently disabled until cci/2 can compile this
#ifdef DISABLED
void __llong_add(unsigned* out, unsigned* a, unsigned* b);
void __llong_sub(unsigned* out, unsigned* a, unsigned* b);
void __llong_mul(unsigned* out, unsigned* a, unsigned* b);
void __llong_divs(unsigned* out, unsigned* a, unsigned* b);
void __llong_divu(unsigned* out, unsigned* a, unsigned* b);
bool __llong_ltu(unsigned* a, unsigned* b);
void __llong_shl(unsigned* out, unsigned* a, int bits);
void __llong_shru(unsigned* out, unsigned* a, int bits);
void __llong_shrs(int* out, int* a, int bits);
void __llong_and(unsigned* out, unsigned* a, unsigned* b);
void __llong_or(unsigned* out, unsigned* a, unsigned* b);
void __llong_xor(unsigned* out, unsigned* a, unsigned* b);
void __llong_not(unsigned* out, unsigned* src);

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
}

static void test_divu(void) {
}

static void test_mods(void) {
}

static void test_modu(void) {
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
            exit(1);
        }
        __llong_shrs((unsigned*)&a, (unsigned*)&b, 32);
        if (a != 0x34567890ULL) {
            exit(1);
        }
        __llong_shrs((unsigned*)&a, (unsigned*)&b, 52);
        if (a != 0x345ULL) {
            exit(1);
        }
    }

    // negative
    {
        unsigned long long a;
        unsigned long long b = 0x9abcdef012345678ULL;
        __llong_shrs((unsigned*)&a, (unsigned*)&b, 0);
        if (a != b) {
            exit(1);
        }
        __llong_shrs((unsigned*)&a, (unsigned*)&b, 4);
        if (a != 0xf9abcdef01234567ULL) {
            exit(1);
        }
        __llong_shrs((unsigned*)&a, (unsigned*)&b, 32);
        if (a != 0xffffffff9abcdef0ULL) {
            exit(1);
        }
        __llong_shrs((unsigned*)&a, (unsigned*)&b, 52);
        if (a != 0xfffffffffffff9abULL) {
            exit(1);
        }
    }
}

static void test_and(void) {
    unsigned long long a = 0x34567890abcdef12ULL;
    unsigned long long b = 0x9abcdef012345678ULL;
    __llong_and((unsigned*)&a, (unsigned*)&a, (unsigned*)&b);
    if (a != 0x1014589002044610ULL) {
        exit(1);
    }
}

static void test_or(void) {
    unsigned long long a = 0x34567890abcdef12ULL;
    unsigned long long b = 0x9abcdef012345678ULL;
    __llong_or((unsigned*)&a, (unsigned*)&a, (unsigned*)&b);
    if (a != 0xbefefef0bbfdff7aULL) {
        exit(1);
    }
}

static void test_xor(void) {
    unsigned long long a = 0x34567890abcdef12ULL;
    unsigned long long b = 0x9abcdef012345678ULL;
    __llong_xor((unsigned*)&a, (unsigned*)&a, (unsigned*)&b);
    if (a != 0xaeeaa660b9f9b96aULL) {
        exit(1);
    }
}

static void test_not(void) {
    unsigned long long a = 0x34567890abcdef12ULL;
    __llong_not((unsigned*)&a, (unsigned*)&a);
    if (a != 0xcba9876f543210edULL) {
        exit(1);
    }
}
#endif

// printf("%llx\n",a);

int main(void) {
    #ifdef DISABLED
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
    test_and();
    test_or();
    test_xor();
    test_not();
    return 0;
    #endif
}
