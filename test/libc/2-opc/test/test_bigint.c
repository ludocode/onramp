// The MIT License (MIT)
// Copyright (c) 2026 Fraser Heavy Software
// This test case is part of the Onramp compiler project.

#ifndef __onramp__ // TODO enable on onramp later

#ifdef __onramp__
#include "bigint.h"
#endif

#ifndef __onramp__
    #include <stdint.h>
    #include <stddef.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    _Noreturn void __fatal(const char* s) {
        fprintf(stderr, "%s\n", s);
        abort();
    }

    // this test case can be compiled standalone with a native C compiler:
    // gcc -Wall -Wextra -Wpedantic -fsanitize=address -g test/test_bigint.c -o /tmp/a && /tmp/a
    #include "../../../../core/libc/2-opc/src/bigint.c"
#endif

/**
 * Verifies that the given bigint matches the given string representation.
 */
void check(uint32_t n, uint32_t* x, const char* str, uint32_t line) {
    char buf[256];
    if (3 + n * 8 > sizeof(buf)) {
        __fatal("need bigger buffer");
    }
    __bigint_print_hex(n, x, buf, sizeof(buf));
    if (0 != strcmp(str, buf)) {
        fprintf(stderr, "failure at line %i:\n", line);
        fprintf(stderr, "    expected: %s\n", str);
        fprintf(stderr, "    actual:   %s\n", buf);
        abort();
    }
}

#define CHECK(n, x, str) check(n, x, str, __LINE__)

static void test_bigint_print(void) {
    uint32_t x[3];

    x[0] = 0;
    x[1] = 0;
    x[2] = 0;
    CHECK(3, x, "0x000000000000000000000000");

    x[0] = 0;
    x[1] = 1;
    x[2] = 0;
    CHECK(3, x, "0x000000000000000100000000");

    x[0] = 0x12345678;
    x[1] = 0x9abcdef0;
    x[2] = 0x87654321;
    CHECK(3, x, "0x876543219abcdef012345678");

    x[0] = 0xffffffff;
    x[1] = 0xffffffff;
    x[2] = 0xffffffff;
    CHECK(3, x, "0xffffffffffffffffffffffff");

    uint32_t y[7];
    y[0] = 0x12345678;
    y[1] = 0x23456789;
    y[2] = 0x3456789a;
    y[3] = 0x456789ab;
    y[4] = 0x56789abc;
    y[5] = 0x6789abcd;
    y[6] = 0x789abcde;
    CHECK(7, y, "0x789abcde6789abcd56789abc456789ab3456789a2345678912345678");
}

static void test_bigint_set_u32(void) {
    uint32_t x[3];

    __bigint_set_u32(3, x, 0);
    CHECK(3, x, "0x000000000000000000000000");

    __bigint_set_u32(3, x, 1);
    CHECK(3, x, "0x000000000000000000000001");

    __bigint_set_u32(3, x, 0x1234abcd);
    CHECK(3, x, "0x00000000000000001234abcd");

    __bigint_set_u32(3, x, 0xffffffff);
    CHECK(3, x, "0x0000000000000000ffffffff");
}

static void test_bigint_add(void) {
    uint32_t x[4];
    uint32_t y[4];
    uint32_t z[4];

    x[0] = 0;
    x[1] = 0;
    x[2] = 0;
    x[3] = 0;
    y[0] = 0;
    y[1] = 0;
    y[2] = 0;
    y[3] = 0;
    __bigint_add(4, z, x, y);
    CHECK(4, z, "0x00000000000000000000000000000000");

    x[0] = 1;
    x[1] = 0;
    x[2] = 0;
    x[3] = 0;
    y[0] = 1;
    y[1] = 0;
    y[2] = 0;
    y[3] = 0;
    __bigint_add(4, z, x, y);
    CHECK(4, z, "0x00000000000000000000000000000002");

    x[0] = 0x80000000;
    x[1] = 0x80000000;
    x[2] = 0x80000000;
    x[3] = 0x80000000;
    y[0] = 0x80000000;
    y[1] = 0x80000000;
    y[2] = 0x80000000;
    y[3] = 0x80000000;
    __bigint_add(4, z, x, y);
    CHECK(4, z, "0x00000001000000010000000100000000");

    x[0] = 0xffffffff;
    x[1] = 0xffffffff;
    x[2] = 0xffffffff;
    x[3] = 0xffffffff;
    y[0] = 0xffffffff;
    y[1] = 0xffffffff;
    y[2] = 0xffffffff;
    y[3] = 0xffffffff;
    __bigint_add(4, z, x, y);
    CHECK(4, z, "0xfffffffffffffffffffffffffffffffe");

    // patterns
    x[0] = 0x12345678;
    x[1] = 0x4567abcd;
    x[2] = 0x89abcdef;
    x[3] = 0xabcdefab;
    y[0] = 0x12345678;
    y[1] = 0x4567abcd;
    y[2] = 0x89abcdef;
    y[3] = 0xabcdefab;
    __bigint_add(4, z, x, y);
    CHECK(4, z, "0x579bdf5713579bde8acf579a2468acf0");

    // random numbers
    x[0] = 0x9efc452f;
    x[1] = 0xc80510eb;
    x[2] = 0x29055749;
    x[3] = 0xab48eb3e;
    y[0] = 0x5cfa0d5a;
    y[1] = 0x89935756;
    y[2] = 0x58e39b80;
    y[3] = 0xf5de01bf;
    __bigint_add(4, z, x, y);
    CHECK(4, z, "0xa126ecfd81e8f2ca51986841fbf65289");
}

static void test_bigint_sub(void) {
    uint32_t x[4];
    uint32_t y[4];
    uint32_t z[4];

    x[0] = 0;
    x[1] = 0;
    x[2] = 0;
    x[3] = 0;
    y[0] = 0;
    y[1] = 0;
    y[2] = 0;
    y[3] = 0;
    __bigint_sub(4, z, x, y);
    CHECK(4, z, "0x00000000000000000000000000000000");

    x[0] = 0;
    x[1] = 0;
    x[2] = 0;
    x[3] = 0;
    y[0] = 1;
    y[1] = 0;
    y[2] = 0;
    y[3] = 0;
    __bigint_sub(4, z, x, y);
    CHECK(4, z, "0xffffffffffffffffffffffffffffffff");

    x[0] = 0;
    x[1] = 0;
    x[2] = 0;
    x[3] = 0;
    y[0] = 0xffffffff;
    y[1] = 0xffffffff;
    y[2] = 0xffffffff;
    y[3] = 0xffffffff;
    __bigint_sub(4, z, x, y);
    CHECK(4, z, "0x00000000000000000000000000000001");

    // matching patterns
    x[0] = 0x12345678;
    x[1] = 0x4567abcd;
    x[2] = 0x89abcdef;
    x[3] = 0xabcdefab;
    y[0] = 0x12345678;
    y[1] = 0x4567abcd;
    y[2] = 0x89abcdef;
    y[3] = 0xabcdefab;
    __bigint_sub(4, z, x, y);
    CHECK(4, z, "0x00000000000000000000000000000000");

    // same random numbers as above
    x[0] = 0x9efc452f;
    x[1] = 0xc80510eb;
    x[2] = 0x29055749;
    x[3] = 0xab48eb3e;
    y[0] = 0x5cfa0d5a;
    y[1] = 0x89935756;
    y[2] = 0x58e39b80;
    y[3] = 0xf5de01bf;
    __bigint_sub(4, z, x, y);
    CHECK(4, z, "0xb56ae97ed021bbc93e71b995420237d5");
}

static void test_bigint_shl(void) {
    uint32_t x[8];
    uint32_t z[8];

    x[0] = 0;
    x[1] = 0;
    x[2] = 0;
    x[3] = 0;
    __bigint_shl(4, z, x, 0);
    CHECK(4, z, "0x00000000000000000000000000000000");
    __bigint_shl(4, z, x, 1);
    CHECK(4, z, "0x00000000000000000000000000000000");
    __bigint_shl(4, z, x, 31);
    CHECK(4, z, "0x00000000000000000000000000000000");
    __bigint_shl(4, z, x, 32);
    CHECK(4, z, "0x00000000000000000000000000000000");
    __bigint_shl(4, z, x, 33);
    CHECK(4, z, "0x00000000000000000000000000000000");
    __bigint_shl(4, z, x, 1000);
    CHECK(4, z, "0x00000000000000000000000000000000");

    x[0] = 0x12345678;
    x[1] = 0x90abcdef;
    __bigint_shl(2, z, x, 0);
    CHECK(2, z, "0x90abcdef12345678");
    __bigint_shl(2, z, x, 1);
    CHECK(2, z, "0x21579bde2468acf0");
    __bigint_shl(2, z, x, 4);
    CHECK(2, z, "0x0abcdef123456780");
    __bigint_shl(2, z, x, 8);
    CHECK(2, z, "0xabcdef1234567800");
    __bigint_shl(2, z, x, 16);
    CHECK(2, z, "0xcdef123456780000");
    __bigint_shl(2, z, x, 31);
    CHECK(2, z, "0x891a2b3c00000000");
    __bigint_shl(2, z, x, 32);
    CHECK(2, z, "0x1234567800000000");
    __bigint_shl(2, z, x, 36);
    CHECK(2, z, "0x2345678000000000");
    __bigint_shl(2, z, x, 60);
    CHECK(2, z, "0x8000000000000000");
    __bigint_shl(2, z, x, 64);
    CHECK(2, z, "0x0000000000000000");

    // same random numbers as above
    x[0] = 0x9efc452f;
    x[1] = 0xc80510eb;
    x[2] = 0x29055749;
    x[3] = 0xab48eb3e;
    x[4] = 0x5cfa0d5a;
    x[5] = 0x89935756;
    x[6] = 0x58e39b80;
    x[7] = 0xf5de01bf;
    CHECK(8, x, "0xf5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb9efc452f");
    __bigint_shl(8, z, x, 0);
    CHECK(8, z, "0xf5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb9efc452f");
    __bigint_shl(8, z, x, 1);
    CHECK(8, z, "0xebbc037eb1c737011326aeacb9f41ab55691d67c520aae93900a21d73df88a5e");
    __bigint_shl(8, z, x, 4);
    CHECK(8, z, "0x5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb9efc452f0");
    __bigint_shl(8, z, x, 8);
    CHECK(8, z, "0xde01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb9efc452f00");
    __bigint_shl(8, z, x, 32);
    CHECK(8, z, "0x58e39b80899357565cfa0d5aab48eb3e29055749c80510eb9efc452f00000000");
    __bigint_shl(8, z, x, 36);
    CHECK(8, z, "0x8e39b80899357565cfa0d5aab48eb3e29055749c80510eb9efc452f000000000");
    __bigint_shl(8, z, x, 60);
    CHECK(8, z, "0x0899357565cfa0d5aab48eb3e29055749c80510eb9efc452f000000000000000");
    __bigint_shl(8, z, x, 64);
    CHECK(8, z, "0x899357565cfa0d5aab48eb3e29055749c80510eb9efc452f0000000000000000");
    __bigint_shl(8, z, x, 124);
    CHECK(8, z, "0xaab48eb3e29055749c80510eb9efc452f0000000000000000000000000000000");
    __bigint_shl(8, z, x, 128);
    CHECK(8, z, "0xab48eb3e29055749c80510eb9efc452f00000000000000000000000000000000");
    __bigint_shl(8, z, x, 132);
    CHECK(8, z, "0xb48eb3e29055749c80510eb9efc452f000000000000000000000000000000000");
    __bigint_shl(8, z, x, 136);
    CHECK(8, z, "0x48eb3e29055749c80510eb9efc452f0000000000000000000000000000000000");
    __bigint_shl(8, z, x, 140);
    CHECK(8, z, "0x8eb3e29055749c80510eb9efc452f00000000000000000000000000000000000");
    __bigint_shl(8, z, x, 141);
    CHECK(8, z, "0x1d67c520aae93900a21d73df88a5e00000000000000000000000000000000000");
    __bigint_shl(8, z, x, 192);
    CHECK(8, z, "0xc80510eb9efc452f000000000000000000000000000000000000000000000000");
    __bigint_shl(8, z, x, 196);
    CHECK(8, z, "0x80510eb9efc452f0000000000000000000000000000000000000000000000000");
    __bigint_shl(8, z, x, 224);
    CHECK(8, z, "0x9efc452f00000000000000000000000000000000000000000000000000000000");
    __bigint_shl(8, z, x, 252);
    CHECK(8, z, "0xf000000000000000000000000000000000000000000000000000000000000000");
    __bigint_shl(8, z, x, 256);
    CHECK(8, z, "0x0000000000000000000000000000000000000000000000000000000000000000");
    __bigint_shl(8, z, x, 0xffffffffu);
    CHECK(8, z, "0x0000000000000000000000000000000000000000000000000000000000000000");
}

static void test_bigint_shru(void) {
    uint32_t x[8];
    uint32_t z[8];

    x[0] = 0;
    x[1] = 0;
    x[2] = 0;
    x[3] = 0;
    __bigint_shru(4, z, x, 0);
    CHECK(4, z, "0x00000000000000000000000000000000");
    __bigint_shru(4, z, x, 1);
    CHECK(4, z, "0x00000000000000000000000000000000");
    __bigint_shru(4, z, x, 31);
    CHECK(4, z, "0x00000000000000000000000000000000");
    __bigint_shru(4, z, x, 32);
    CHECK(4, z, "0x00000000000000000000000000000000");
    __bigint_shru(4, z, x, 33);
    CHECK(4, z, "0x00000000000000000000000000000000");
    __bigint_shru(4, z, x, 1000);
    CHECK(4, z, "0x00000000000000000000000000000000");

    x[0] = 0x12345678;
    x[1] = 0x90abcdef;
    __bigint_shru(2, z, x, 0);
    CHECK(2, z, "0x90abcdef12345678");
    __bigint_shru(2, z, x, 1);
    CHECK(2, z, "0x4855e6f7891a2b3c");
    __bigint_shru(2, z, x, 4);
    CHECK(2, z, "0x090abcdef1234567");
    __bigint_shru(2, z, x, 32);
    CHECK(2, z, "0x0000000090abcdef");
    __bigint_shru(2, z, x, 36);
    CHECK(2, z, "0x00000000090abcde");
    __bigint_shru(2, z, x, 60);
    CHECK(2, z, "0x0000000000000009");
    __bigint_shru(2, z, x, 63);
    CHECK(2, z, "0x0000000000000001");
    __bigint_shru(2, z, x, 64);
    CHECK(2, z, "0x0000000000000000");

    // same random numbers as above
    x[0] = 0x9efc452f;
    x[1] = 0xc80510eb;
    x[2] = 0x29055749;
    x[3] = 0xab48eb3e;
    x[4] = 0x5cfa0d5a;
    x[5] = 0x89935756;
    x[6] = 0x58e39b80;
    x[7] = 0xf5de01bf;
    CHECK(8, x, "0xf5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb9efc452f");
    __bigint_shru(8, z, x, 0);
    CHECK(8, z, "0xf5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb9efc452f");
    __bigint_shru(8, z, x, 1);
    CHECK(8, z, "0x7aef00dfac71cdc044c9abab2e7d06ad55a4759f1482aba4e4028875cf7e2297");
    __bigint_shru(8, z, x, 4);
    CHECK(8, z, "0x0f5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb9efc452");
    __bigint_shru(8, z, x, 8);
    CHECK(8, z, "0x00f5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb9efc45");
    __bigint_shru(8, z, x, 28);
    CHECK(8, z, "0x0000000f5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb9");
    __bigint_shru(8, z, x, 32);
    CHECK(8, z, "0x00000000f5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb");
    __bigint_shru(8, z, x, 32);
    CHECK(8, z, "0x00000000f5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510eb");
    __bigint_shru(8, z, x, 36);
    CHECK(8, z, "0x000000000f5de01bf58e39b80899357565cfa0d5aab48eb3e29055749c80510e");
    __bigint_shru(8, z, x, 128);
    CHECK(8, z, "0x00000000000000000000000000000000f5de01bf58e39b80899357565cfa0d5a");
    __bigint_shru(8, z, x, 132);
    CHECK(8, z, "0x000000000000000000000000000000000f5de01bf58e39b80899357565cfa0d5");
    __bigint_shru(8, z, x, 164);
    CHECK(8, z, "0x00000000000000000000000000000000000000000f5de01bf58e39b808993575");
    __bigint_shru(8, z, x, 252);
    CHECK(8, z, "0x000000000000000000000000000000000000000000000000000000000000000f");
    __bigint_shru(8, z, x, 256);
    CHECK(8, z, "0x0000000000000000000000000000000000000000000000000000000000000000");
    __bigint_shru(8, z, x, 0xffffffffu);
    CHECK(8, z, "0x0000000000000000000000000000000000000000000000000000000000000000");
}

#endif

int main(void) {
#ifndef __onramp__ // TODO enable on onramp later
    test_bigint_print();
    test_bigint_set_u32();
    test_bigint_add();
    test_bigint_sub();
    test_bigint_shl();
    test_bigint_shru();
#endif
}

